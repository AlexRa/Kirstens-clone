/** 
 * @file llfloaterproperties.cpp
 * @brief A floater which shows an inventory item's properties.
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"
#include "llfloaterproperties.h"

#include <algorithm>
#include <functional>
#include "llcachename.h"
#include "lldbstrings.h"
#include "llfloaterreg.h"
#include "llinventory.h"

#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llavataractions.h"
#include "llinventoryobserver.h"
#include "llinventorymodel.h"
#include "lllineeditor.h"
//#include "llspinctrl.h"
#include "llradiogroup.h"
#include "llresmgr.h"
#include "roles_constants.h"
#include "llselectmgr.h"
#include "lltextbox.h"
#include "lltrans.h"
#include "lluiconstants.h"
#include "llviewerinventory.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llgroupactions.h"

#include "lluictrlfactory.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLPropertiesObserver
//
// helper class to watch the inventory. 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Ugh. This can't be a singleton because it needs to remove itself
//  from the inventory observer list when destroyed, which could
//  happen after gInventory has already been destroyed if a singleton.
// Instead, do our own ref counting and create / destroy it as needed
class LLPropertiesObserver : public LLInventoryObserver
{
public:
	LLPropertiesObserver(LLFloaterProperties* floater)
		: mFloater(floater)
	{
		gInventory.addObserver(this);
	}
	virtual ~LLPropertiesObserver()
	{
		gInventory.removeObserver(this);
	}
	virtual void changed(U32 mask);
private:
	LLFloaterProperties* mFloater;
};

void LLPropertiesObserver::changed(U32 mask)
{
	// if there's a change we're interested in.
	if((mask & (LLInventoryObserver::LABEL | LLInventoryObserver::INTERNAL | LLInventoryObserver::REMOVE)) != 0)
	{
		mFloater->dirty();
	}
}



///----------------------------------------------------------------------------
/// Class LLFloaterProperties
///----------------------------------------------------------------------------

// Default constructor
LLFloaterProperties::LLFloaterProperties(const LLUUID& item_id)
  : LLFloater(mItemID),
	mItemID(item_id),
	mDirty(TRUE)
{
	mPropertiesObserver = new LLPropertiesObserver(this);
	
	//LLUICtrlFactory::getInstance()->buildFloater(this,"floater_inventory_item_properties.xml");
}

// Destroys the object
LLFloaterProperties::~LLFloaterProperties()
{
	delete mPropertiesObserver;
	mPropertiesObserver = NULL;
}

// virtual
BOOL LLFloaterProperties::postBuild()
{
	// build the UI
	// item name & description
	childSetPrevalidate("LabelItemName",&LLTextValidate::validateASCIIPrintableNoPipe);
	getChild<LLUICtrl>("LabelItemName")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitName,this));
	childSetPrevalidate("LabelItemDesc",&LLTextValidate::validateASCIIPrintableNoPipe);
	getChild<LLUICtrl>("LabelItemDesc")->setCommitCallback(boost::bind(&LLFloaterProperties:: onCommitDescription, this));
	// Creator information
	getChild<LLUICtrl>("BtnCreator")->setCommitCallback(boost::bind(&LLFloaterProperties::onClickCreator,this));
	// owner information
	getChild<LLUICtrl>("BtnOwner")->setCommitCallback(boost::bind(&LLFloaterProperties::onClickOwner,this));
	// acquired date
	// owner permissions
	// Permissions debug text
	// group permissions
	getChild<LLUICtrl>("CheckShareWithGroup")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitPermissions, this));
	// everyone permissions
	getChild<LLUICtrl>("CheckEveryoneCopy")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitPermissions, this));
	// next owner permissions
	getChild<LLUICtrl>("CheckNextOwnerModify")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitPermissions, this));
	getChild<LLUICtrl>("CheckNextOwnerCopy")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitPermissions, this));
	getChild<LLUICtrl>("CheckNextOwnerTransfer")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitPermissions, this));
	// Mark for sale or not, and sale info
	getChild<LLUICtrl>("CheckPurchase")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitSaleInfo, this));
	getChild<LLUICtrl>("RadioSaleType")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitSaleType, this));
	// "Price" label for edit
	getChild<LLUICtrl>("Edit Cost")->setCommitCallback(boost::bind(&LLFloaterProperties::onCommitSaleInfo, this));
	// The UI has been built, now fill in all the values
	refresh();

	return TRUE;
}

// virtual
void LLFloaterProperties::onOpen(const LLSD& key)
{
	refresh();
}

void LLFloaterProperties::refresh()
{
	LLInventoryItem* item = findItem();
	if(item)
	{
		refreshFromItem(item);
	}
	else
	{
		//RN: it is possible that the container object is in the middle of an inventory refresh
		// causing findItem() to fail, so just temporarily disable everything
		
		mDirty = TRUE;

		const char* enableNames[]={
			"LabelItemName",
			"LabelItemDesc",
			"LabelCreatorName",
			"BtnCreator",
			"LabelOwnerName",
			"BtnOwner",
			"CheckOwnerModify",
			"CheckOwnerCopy",
			"CheckOwnerTransfer",
			"CheckShareWithGroup",
			"CheckEveryoneCopy",
			"CheckNextOwnerModify",
			"CheckNextOwnerCopy",
			"CheckNextOwnerTransfer",
			"CheckPurchase",
			"RadioSaleType",
			"Edit Cost"
		};
		for(size_t t=0; t<LL_ARRAY_SIZE(enableNames); ++t)
		{
			childSetEnabled(enableNames[t],false);
		}
		const char* hideNames[]={
			"BaseMaskDebug",
			"OwnerMaskDebug",
			"GroupMaskDebug",
			"EveryoneMaskDebug",
			"NextMaskDebug"
		};
		for(size_t t=0; t<LL_ARRAY_SIZE(hideNames); ++t)
		{
			childSetVisible(hideNames[t],false);
		}
	}
}

void LLFloaterProperties::draw()
{
	if (mDirty)
	{
		// RN: clear dirty first because refresh can set dirty to TRUE
		mDirty = FALSE;
		refresh();
	}

	LLFloater::draw();
}

void LLFloaterProperties::refreshFromItem(LLInventoryItem* item)
{
	////////////////////////
	// PERMISSIONS LOOKUP //
	////////////////////////

	// do not enable the UI for incomplete items.
	LLViewerInventoryItem* i = (LLViewerInventoryItem*)item;
	BOOL is_complete = i->isComplete();
	const BOOL cannot_restrict_permissions = LLInventoryType::cannotRestrictPermissions(i->getInventoryType());
	const BOOL is_calling_card = (i->getInventoryType() == LLInventoryType::IT_CALLINGCARD);
	const LLPermissions& perm = item->getPermissions();
	const BOOL can_agent_manipulate = gAgent.allowOperation(PERM_OWNER, perm, 
															GP_OBJECT_MANIPULATE);
	const BOOL can_agent_sell = gAgent.allowOperation(PERM_OWNER, perm, 
													  GP_OBJECT_SET_SALE) &&
		!cannot_restrict_permissions;
	const BOOL is_link = i->getIsLinkType();

	// You need permission to modify the object to modify an inventory
	// item in it.
	LLViewerObject* object = NULL;
	if(!mObjectID.isNull()) object = gObjectList.findObject(mObjectID);
	BOOL is_obj_modify = TRUE;
	if(object)
	{
		is_obj_modify = object->permOwnerModify();
	}

	//////////////////////
	// ITEM NAME & DESC //
	//////////////////////
	BOOL is_modifiable = gAgent.allowOperation(PERM_MODIFY, perm,
											   GP_OBJECT_MANIPULATE)
		&& is_obj_modify && is_complete;

	childSetEnabled("LabelItemNameTitle",TRUE);
	childSetEnabled("LabelItemName",is_modifiable && !is_calling_card); // for now, don't allow rename of calling cards
	childSetText("LabelItemName",item->getName());
	childSetEnabled("LabelItemDescTitle",TRUE);
	childSetEnabled("LabelItemDesc",is_modifiable);
	childSetVisible("IconLocked",!is_modifiable);
	childSetText("LabelItemDesc",item->getDescription());

	//////////////////
	// CREATOR NAME //
	//////////////////
	if(!gCacheName) return;
	if(!gAgent.getRegion()) return;

	if (item->getCreatorUUID().notNull())
	{
		std::string name;
		gCacheName->getFullName(item->getCreatorUUID(), name);
		childSetEnabled("BtnCreator",TRUE);
		childSetEnabled("LabelCreatorTitle",TRUE);
		childSetEnabled("LabelCreatorName",TRUE);
		childSetText("LabelCreatorName",name);
	}
	else
	{
		childSetEnabled("BtnCreator",FALSE);
		childSetEnabled("LabelCreatorTitle",FALSE);
		childSetEnabled("LabelCreatorName",FALSE);
		childSetText("LabelCreatorName",getString("unknown"));
	}

	////////////////
	// OWNER NAME //
	////////////////
	if(perm.isOwned())
	{
		std::string name;
		if (perm.isGroupOwned())
		{
			gCacheName->getGroupName(perm.getGroup(), name);
		}
		else
		{
			gCacheName->getFullName(perm.getOwner(), name);
		}
		childSetEnabled("BtnOwner",TRUE);
		childSetEnabled("LabelOwnerTitle",TRUE);
		childSetEnabled("LabelOwnerName",TRUE);
		childSetText("LabelOwnerName",name);
	}
	else
	{
		childSetEnabled("BtnOwner",FALSE);
		childSetEnabled("LabelOwnerTitle",FALSE);
		childSetEnabled("LabelOwnerName",FALSE);
		childSetText("LabelOwnerName",getString("public"));
	}
	
	//////////////////
	// ACQUIRE DATE //
	//////////////////
	
	time_t time_utc = item->getCreationDate();
	if (0 == time_utc)
	{
		childSetText("LabelAcquiredDate",getString("unknown"));
	}
	else
	{
		std::string timeStr = getString("acquiredDate");
		LLSD substitution;
		substitution["datetime"] = (S32) time_utc;
		LLStringUtil::format (timeStr, substitution);
		childSetText ("LabelAcquiredDate", timeStr);
	}

	///////////////////////
	// OWNER PERMISSIONS //
	///////////////////////
	if(can_agent_manipulate)
	{
		childSetText("OwnerLabel",getString("you_can"));
	}
	else
	{
		childSetText("OwnerLabel",getString("owner_can"));
	}

	U32 base_mask		= perm.getMaskBase();
	U32 owner_mask		= perm.getMaskOwner();
	U32 group_mask		= perm.getMaskGroup();
	U32 everyone_mask	= perm.getMaskEveryone();
	U32 next_owner_mask	= perm.getMaskNextOwner();

	childSetEnabled("OwnerLabel",TRUE);
	childSetEnabled("CheckOwnerModify",FALSE);
	childSetValue("CheckOwnerModify",LLSD((BOOL)(owner_mask & PERM_MODIFY)));
	childSetEnabled("CheckOwnerCopy",FALSE);
	childSetValue("CheckOwnerCopy",LLSD((BOOL)(owner_mask & PERM_COPY)));
	childSetEnabled("CheckOwnerTransfer",FALSE);
	childSetValue("CheckOwnerTransfer",LLSD((BOOL)(owner_mask & PERM_TRANSFER)));

	///////////////////////
	// DEBUG PERMISSIONS //
	///////////////////////

	if( gSavedSettings.getBOOL("DebugPermissions") )
	{
		BOOL slam_perm 			= FALSE;
		BOOL overwrite_group	= FALSE;
		BOOL overwrite_everyone	= FALSE;

		if (item->getType() == LLAssetType::AT_OBJECT)
		{
			U32 flags = item->getFlags();
			slam_perm 			= flags & LLInventoryItem::II_FLAGS_OBJECT_SLAM_PERM;
			overwrite_everyone	= flags & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_EVERYONE;
			overwrite_group		= flags & LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_GROUP;
		}
		
		std::string perm_string;

		perm_string = "B: ";
		perm_string += mask_to_string(base_mask);
		childSetText("BaseMaskDebug",perm_string);
		childSetVisible("BaseMaskDebug",TRUE);
		
		perm_string = "O: ";
		perm_string += mask_to_string(owner_mask);
		childSetText("OwnerMaskDebug",perm_string);
		childSetVisible("OwnerMaskDebug",TRUE);
		
		perm_string = "G";
		perm_string += overwrite_group ? "*: " : ": ";
		perm_string += mask_to_string(group_mask);
		childSetText("GroupMaskDebug",perm_string);
		childSetVisible("GroupMaskDebug",TRUE);
		
		perm_string = "E";
		perm_string += overwrite_everyone ? "*: " : ": ";
		perm_string += mask_to_string(everyone_mask);
		childSetText("EveryoneMaskDebug",perm_string);
		childSetVisible("EveryoneMaskDebug",TRUE);
		
		perm_string = "N";
		perm_string += slam_perm ? "*: " : ": ";
		perm_string += mask_to_string(next_owner_mask);
		childSetText("NextMaskDebug",perm_string);
		childSetVisible("NextMaskDebug",TRUE);
	}
	else
	{
		childSetVisible("BaseMaskDebug",FALSE);
		childSetVisible("OwnerMaskDebug",FALSE);
		childSetVisible("GroupMaskDebug",FALSE);
		childSetVisible("EveryoneMaskDebug",FALSE);
		childSetVisible("NextMaskDebug",FALSE);
	}

	/////////////
	// SHARING //
	/////////////

	// Check for ability to change values.
	if (is_link || cannot_restrict_permissions)
	{
		childSetEnabled("CheckShareWithGroup",FALSE);
		childSetEnabled("CheckEveryoneCopy",FALSE);
	}
	else if (is_obj_modify && can_agent_manipulate)
	{
		childSetEnabled("CheckShareWithGroup",TRUE);
		childSetEnabled("CheckEveryoneCopy",(owner_mask & PERM_COPY) && (owner_mask & PERM_TRANSFER));
	}
	else
	{
		childSetEnabled("CheckShareWithGroup",FALSE);
		childSetEnabled("CheckEveryoneCopy",FALSE);
	}

	// Set values.
	BOOL is_group_copy = (group_mask & PERM_COPY) ? TRUE : FALSE;
	BOOL is_group_modify = (group_mask & PERM_MODIFY) ? TRUE : FALSE;
	BOOL is_group_move = (group_mask & PERM_MOVE) ? TRUE : FALSE;

	if (is_group_copy && is_group_modify && is_group_move)
	{
		childSetValue("CheckShareWithGroup",LLSD((BOOL)TRUE));

		LLCheckBoxCtrl* ctl = getChild<LLCheckBoxCtrl>("CheckShareWithGroup");
		if(ctl)
		{
			ctl->setTentative(FALSE);
		}
	}
	else if (!is_group_copy && !is_group_modify && !is_group_move)
	{
		childSetValue("CheckShareWithGroup",LLSD((BOOL)FALSE));
		LLCheckBoxCtrl* ctl = getChild<LLCheckBoxCtrl>("CheckShareWithGroup");
		if(ctl)
		{
			ctl->setTentative(FALSE);
		}
	}
	else
	{
		LLCheckBoxCtrl* ctl = getChild<LLCheckBoxCtrl>("CheckShareWithGroup");
		if(ctl)
		{
			ctl->setTentative(TRUE);
			ctl->set(TRUE);
		}
	}
	
	childSetValue("CheckEveryoneCopy",LLSD((BOOL)(everyone_mask & PERM_COPY)));

	///////////////
	// SALE INFO //
	///////////////

	const LLSaleInfo& sale_info = item->getSaleInfo();
	BOOL is_for_sale = sale_info.isForSale();
	// Check for ability to change values.
	if (is_obj_modify && can_agent_sell 
		&& gAgent.allowOperation(PERM_TRANSFER, perm, GP_OBJECT_MANIPULATE))
	{
		childSetEnabled("SaleLabel",is_complete);
		childSetEnabled("CheckPurchase",is_complete);

		childSetEnabled("NextOwnerLabel",TRUE);
		childSetEnabled("CheckNextOwnerModify",(base_mask & PERM_MODIFY) && !cannot_restrict_permissions);
		childSetEnabled("CheckNextOwnerCopy",(base_mask & PERM_COPY) && !cannot_restrict_permissions);
		childSetEnabled("CheckNextOwnerTransfer",(next_owner_mask & PERM_COPY) && !cannot_restrict_permissions);

		childSetEnabled("RadioSaleType",is_complete && is_for_sale);
		childSetEnabled("TextPrice",is_complete && is_for_sale);
		childSetEnabled("Edit Cost",is_complete && is_for_sale);
	}
	else
	{
		childSetEnabled("SaleLabel",FALSE);
		childSetEnabled("CheckPurchase",FALSE);

		childSetEnabled("NextOwnerLabel",FALSE);
		childSetEnabled("CheckNextOwnerModify",FALSE);
		childSetEnabled("CheckNextOwnerCopy",FALSE);
		childSetEnabled("CheckNextOwnerTransfer",FALSE);

		childSetEnabled("RadioSaleType",FALSE);
		childSetEnabled("TextPrice",FALSE);
		childSetEnabled("Edit Cost",FALSE);
	}

	// Set values.
	childSetValue("CheckPurchase", is_for_sale);
	childSetEnabled("combobox sale copy", is_for_sale);
	childSetEnabled("Edit Cost", is_for_sale);
	childSetValue("CheckNextOwnerModify",LLSD(BOOL(next_owner_mask & PERM_MODIFY)));
	childSetValue("CheckNextOwnerCopy",LLSD(BOOL(next_owner_mask & PERM_COPY)));
	childSetValue("CheckNextOwnerTransfer",LLSD(BOOL(next_owner_mask & PERM_TRANSFER)));

	LLRadioGroup* radioSaleType = getChild<LLRadioGroup>("RadioSaleType");
	if (is_for_sale)
	{
		radioSaleType->setSelectedIndex((S32)sale_info.getSaleType() - 1);
		S32 numerical_price;
		numerical_price = sale_info.getSalePrice();
		childSetText("Edit Cost",llformat("%d",numerical_price));
	}
	else
	{
		radioSaleType->setSelectedIndex(-1);
		childSetText("Edit Cost",llformat("%d",0));
	}
}

void LLFloaterProperties::onClickCreator()
{
	LLInventoryItem* item = findItem();
	if(!item) return;
	if(!item->getCreatorUUID().isNull())
	{
		LLAvatarActions::showProfile(item->getCreatorUUID());
	}
}

// static
void LLFloaterProperties::onClickOwner()
{
	LLInventoryItem* item = findItem();
	if(!item) return;
	if(item->getPermissions().isGroupOwned())
	{
		LLGroupActions::show(item->getPermissions().getGroup());
	}
	else
	{
		LLAvatarActions::showProfile(item->getPermissions().getOwner());
	}
}

// static
void LLFloaterProperties::onCommitName()
{
	//llinfos << "LLFloaterProperties::onCommitName()" << llendl;
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)findItem();
	if(!item)
	{
		return;
	}
	LLLineEditor* labelItemName = getChild<LLLineEditor>("LabelItemName");

	if(labelItemName&&
	   (item->getName() != labelItemName->getText()) && 
	   (gAgent.allowOperation(PERM_MODIFY, item->getPermissions(), GP_OBJECT_MANIPULATE)) )
	{
		LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem(item);
		new_item->rename(labelItemName->getText());
		if(mObjectID.isNull())
		{
			new_item->updateServer(FALSE);
			gInventory.updateItem(new_item);
			gInventory.notifyObservers();
		}
		else
		{
			LLViewerObject* object = gObjectList.findObject(mObjectID);
			if(object)
			{
				object->updateInventory(
					new_item,
					TASK_INVENTORY_ITEM_KEY,
					false);
			}
		}
	}
}

void LLFloaterProperties::onCommitDescription()
{
	//llinfos << "LLFloaterProperties::onCommitDescription()" << llendl;
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)findItem();
	if(!item) return;

	LLLineEditor* labelItemDesc = getChild<LLLineEditor>("LabelItemDesc");
	if(!labelItemDesc)
	{
		return;
	}
	if((item->getDescription() != labelItemDesc->getText()) && 
	   (gAgent.allowOperation(PERM_MODIFY, item->getPermissions(), GP_OBJECT_MANIPULATE)))
	{
		LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem(item);

		new_item->setDescription(labelItemDesc->getText());
		if(mObjectID.isNull())
		{
			new_item->updateServer(FALSE);
			gInventory.updateItem(new_item);
			gInventory.notifyObservers();
		}
		else
		{
			LLViewerObject* object = gObjectList.findObject(mObjectID);
			if(object)
			{
				object->updateInventory(
					new_item,
					TASK_INVENTORY_ITEM_KEY,
					false);
			}
		}
	}
}

// static
void LLFloaterProperties::onCommitPermissions()
{
	//llinfos << "LLFloaterProperties::onCommitPermissions()" << llendl;
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)findItem();
	if(!item) return;
	LLPermissions perm(item->getPermissions());


	LLCheckBoxCtrl* CheckShareWithGroup = getChild<LLCheckBoxCtrl>("CheckShareWithGroup");

	if(CheckShareWithGroup)
	{
		perm.setGroupBits(gAgent.getID(), gAgent.getGroupID(),
						CheckShareWithGroup->get(),
						PERM_MODIFY | PERM_MOVE | PERM_COPY);
	}
	LLCheckBoxCtrl* CheckEveryoneCopy = getChild<LLCheckBoxCtrl>("CheckEveryoneCopy");
	if(CheckEveryoneCopy)
	{
		perm.setEveryoneBits(gAgent.getID(), gAgent.getGroupID(),
						 CheckEveryoneCopy->get(), PERM_COPY);
	}

	LLCheckBoxCtrl* CheckNextOwnerModify = getChild<LLCheckBoxCtrl>("CheckNextOwnerModify");
	if(CheckNextOwnerModify)
	{
		perm.setNextOwnerBits(gAgent.getID(), gAgent.getGroupID(),
							CheckNextOwnerModify->get(), PERM_MODIFY);
	}
	LLCheckBoxCtrl* CheckNextOwnerCopy = getChild<LLCheckBoxCtrl>("CheckNextOwnerCopy");
	if(CheckNextOwnerCopy)
	{
		perm.setNextOwnerBits(gAgent.getID(), gAgent.getGroupID(),
							CheckNextOwnerCopy->get(), PERM_COPY);
	}
	LLCheckBoxCtrl* CheckNextOwnerTransfer = getChild<LLCheckBoxCtrl>("CheckNextOwnerTransfer");
	if(CheckNextOwnerTransfer)
	{
		perm.setNextOwnerBits(gAgent.getID(), gAgent.getGroupID(),
							CheckNextOwnerTransfer->get(), PERM_TRANSFER);
	}
	if(perm != item->getPermissions()
		&& item->isComplete())
	{
		LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem(item);
		new_item->setPermissions(perm);
		U32 flags = new_item->getFlags();
		// If next owner permissions have changed (and this is an object)
		// then set the slam permissions flag so that they are applied on rez.
		if((perm.getMaskNextOwner()!=item->getPermissions().getMaskNextOwner())
		   && (item->getType() == LLAssetType::AT_OBJECT))
		{
			flags |= LLInventoryItem::II_FLAGS_OBJECT_SLAM_PERM;
		}
		// If everyone permissions have changed (and this is an object)
		// then set the overwrite everyone permissions flag so they
		// are applied on rez.
		if ((perm.getMaskEveryone()!=item->getPermissions().getMaskEveryone())
			&& (item->getType() == LLAssetType::AT_OBJECT))
		{
			flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_EVERYONE;
		}
		// If group permissions have changed (and this is an object)
		// then set the overwrite group permissions flag so they
		// are applied on rez.
		if ((perm.getMaskGroup()!=item->getPermissions().getMaskGroup())
			&& (item->getType() == LLAssetType::AT_OBJECT))
		{
			flags |= LLInventoryItem::II_FLAGS_OBJECT_PERM_OVERWRITE_GROUP;
		}
		new_item->setFlags(flags);
		if(mObjectID.isNull())
		{
			new_item->updateServer(FALSE);
			gInventory.updateItem(new_item);
			gInventory.notifyObservers();
		}
		else
		{
			LLViewerObject* object = gObjectList.findObject(mObjectID);
			if(object)
			{
				object->updateInventory(
					new_item,
					TASK_INVENTORY_ITEM_KEY,
					false);
			}
		}
	}
	else
	{
		// need to make sure we don't just follow the click
		refresh();
	}
}

// static
void LLFloaterProperties::onCommitSaleInfo()
{
	//llinfos << "LLFloaterProperties::onCommitSaleInfo()" << llendl;
	updateSaleInfo();
}

// static
void LLFloaterProperties::onCommitSaleType()
{
	//llinfos << "LLFloaterProperties::onCommitSaleType()" << llendl;
	updateSaleInfo();
}

void LLFloaterProperties::updateSaleInfo()
{
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)findItem();
	if(!item) return;
	LLSaleInfo sale_info(item->getSaleInfo());
	if(!gAgent.allowOperation(PERM_TRANSFER, item->getPermissions(), GP_OBJECT_SET_SALE))
	{
		childSetValue("CheckPurchase",LLSD((BOOL)FALSE));
	}

	if((BOOL)childGetValue("CheckPurchase"))
	{
		// turn on sale info
		LLSaleInfo::EForSale sale_type = LLSaleInfo::FS_COPY;
	
		LLRadioGroup* RadioSaleType = getChild<LLRadioGroup>("RadioSaleType");
		if(RadioSaleType)
		{
			switch (RadioSaleType->getSelectedIndex())
			{
			case 0:
				sale_type = LLSaleInfo::FS_ORIGINAL;
				break;
			case 1:
				sale_type = LLSaleInfo::FS_COPY;
				break;
			case 2:
				sale_type = LLSaleInfo::FS_CONTENTS;
				break;
			default:
				sale_type = LLSaleInfo::FS_COPY;
				break;
			}
		}

		if (sale_type == LLSaleInfo::FS_COPY 
			&& !gAgent.allowOperation(PERM_COPY, item->getPermissions(), 
									  GP_OBJECT_SET_SALE))
		{
			sale_type = LLSaleInfo::FS_ORIGINAL;
		}

	     
		
		S32 price = -1;
		price =  getChild<LLUICtrl>("Edit Cost")->getValue().asInteger();;

		// Invalid data - turn off the sale
		if (price < 0)
		{
			sale_type = LLSaleInfo::FS_NOT;
			price = 0;
		}

		sale_info.setSaleType(sale_type);
		sale_info.setSalePrice(price);
	}
	else
	{
		sale_info.setSaleType(LLSaleInfo::FS_NOT);
	}
	if(sale_info != item->getSaleInfo()
		&& item->isComplete())
	{
		LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem(item);

		// Force an update on the sale price at rez
		if (item->getType() == LLAssetType::AT_OBJECT)
		{
			U32 flags = new_item->getFlags();
			flags |= LLInventoryItem::II_FLAGS_OBJECT_SLAM_SALE;
			new_item->setFlags(flags);
		}

		new_item->setSaleInfo(sale_info);
		if(mObjectID.isNull())
		{
			// This is in the agent's inventory.
			new_item->updateServer(FALSE);
			gInventory.updateItem(new_item);
			gInventory.notifyObservers();
		}
		else
		{
			// This is in an object's contents.
			LLViewerObject* object = gObjectList.findObject(mObjectID);
			if(object)
			{
				object->updateInventory(
					new_item,
					TASK_INVENTORY_ITEM_KEY,
					false);
			}
		}
	}
	else
	{
		// need to make sure we don't just follow the click
		refresh();
	}
}

LLInventoryItem* LLFloaterProperties::findItem() const
{
	LLInventoryItem* item = NULL;
	if(mObjectID.isNull())
	{
		// it is in agent inventory
		item = gInventory.getItem(mItemID);
	}
	else
	{
		LLViewerObject* object = gObjectList.findObject(mObjectID);
		if(object)
		{
			item = (LLInventoryItem*)object->getInventoryObject(mItemID);
		}
	}
	return item;
}

//static
void LLFloaterProperties::dirtyAll()
{
	LLFloaterReg::const_instance_list_t& inst_list = LLFloaterReg::getFloaterList("properties");
	for (LLFloaterReg::const_instance_list_t::const_iterator iter = inst_list.begin();
		 iter != inst_list.end(); ++iter)
	{
		LLFloaterProperties* floater = dynamic_cast<LLFloaterProperties*>(*iter);
		llassert(floater); // else cast failed - wrong type D:
		if (floater)
		{
			floater->dirty();
		}
	}
}

///----------------------------------------------------------------------------
/// LLMultiProperties
///----------------------------------------------------------------------------

LLMultiProperties::LLMultiProperties()
	: LLMultiFloater(LLSD())
{
	// *TODO: There should be a .xml file for this
	const LLRect& nextrect = LLFloaterReg::getFloaterRect("properties"); // place where the next properties should show up
	if (nextrect.getWidth() > 0)
	{
		setRect(nextrect);
	}
	else
	{
		// start with a small rect in the top-left corner ; will get resized
		LLRect rect;
		rect.setLeftTopAndSize(0, gViewerWindow->getWindowHeightScaled(), 20, 20);
		setRect(rect);
	}
	setTitle(LLTrans::getString("MultiPropertiesTitle"));
	buildTabContainer();
}

///----------------------------------------------------------------------------
/// Local function definitions
///----------------------------------------------------------------------------
