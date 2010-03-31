/** 
 * @file llfloaterbuy.cpp
 * @author James Cook
 * @brief LLFloaterBuy class implementation
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 * 
 */

/**
 * Floater that appears when buying an object, giving a preview
 * of its contents and their permissions.
 */

#include "llviewerprecompiledheaders.h"

#include "llfloaterbuy.h"

#include "llagent.h"			// for agent id
#include "llinventorymodel.h"	// for gInventory
#include "llfloaterreg.h"
#include "llfloaterinventory.h"	// for get_item_icon
#include "llinventoryfunctions.h"
#include "llnotificationsutil.h"
#include "llselectmgr.h"
#include "llscrolllistctrl.h"
#include "llviewerobject.h"
#include "lluictrlfactory.h"
#include "llviewerwindow.h"
#include "lltrans.h"

LLFloaterBuy::LLFloaterBuy(const LLSD& key)
:	LLFloater(key)
{
// 	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_buy_object.xml");
}

BOOL LLFloaterBuy::postBuild()
{
	childDisable("object_list");
	childDisable("item_list");

	getChild<LLUICtrl>("cancel_btn")->setCommitCallback( boost::bind(&LLFloaterBuy::onClickCancel, this));
	getChild<LLUICtrl>("buy_btn")->setCommitCallback( boost::bind(&LLFloaterBuy::onClickBuy, this));

	setDefaultBtn("cancel_btn"); // to avoid accidental buy (SL-43130)
	
	// Always center the dialog.  User can change the size,
	// but purchases are important and should be center screen.
	// This also avoids problems where the user resizes the application window
	// mid-session and the saved rect is off-center.
	center();

	return TRUE;
}

LLFloaterBuy::~LLFloaterBuy()
{
	mObjectSelection = NULL;
}

void LLFloaterBuy::reset()
{
	LLScrollListCtrl* object_list = getChild<LLScrollListCtrl>("object_list");
	if (object_list) object_list->deleteAllItems();

	LLScrollListCtrl* item_list = getChild<LLScrollListCtrl>("item_list");
	if (item_list) item_list->deleteAllItems();
}

// static
void LLFloaterBuy::show(const LLSaleInfo& sale_info)
{
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();

	if (selection->getRootObjectCount() != 1)
	{
		LLNotificationsUtil::add("BuyOneObjectOnly");
		return;
	}
	
	LLFloaterBuy* floater = LLFloaterReg::showTypedInstance<LLFloaterBuy>("buy_object");
	if (!floater)
		return;
	
	// Clean up the lists...
	floater->reset();
	floater->mSaleInfo = sale_info;
	floater->mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	
	LLSelectNode* node = selection->getFirstRootNode();
	if (!node)
		return;
	
	// Set title based on sale type
	LLUIString title;
	switch (sale_info.getSaleType())
	{
	  case LLSaleInfo::FS_ORIGINAL:
		title = floater->getString("title_buy_text");
		break;
	  case LLSaleInfo::FS_COPY:
	  default:
		title = floater->getString("title_buy_copy_text");
		break;
	}
	title.setArg("[NAME]", node->mName);
	floater->setTitle(title);

	LLUUID owner_id;
	std::string owner_name;
	BOOL owners_identical = LLSelectMgr::getInstance()->selectGetOwner(owner_id, owner_name);
	if (!owners_identical)
	{
		LLNotificationsUtil::add("BuyObjectOneOwner");
		return;
	}

	LLCtrlListInterface *object_list = floater->childGetListInterface("object_list");
	if (!object_list)
	{
		return;
	}

	// Update the display
	// Display next owner permissions
	LLSD row;

	// Compute icon for this item
	std::string icon_name = get_item_icon_name(LLAssetType::AT_OBJECT, 
									 LLInventoryType::IT_OBJECT,
									 0x0, FALSE);

	row["columns"][0]["column"] = "icon";
	row["columns"][0]["type"] = "icon";
	row["columns"][0]["value"] = icon_name;
	
	// Append the permissions that you will acquire (not the current
	// permissions).
	U32 next_owner_mask = node->mPermissions->getMaskNextOwner();
	std::string text = node->mName;
	if (!(next_owner_mask & PERM_COPY))
	{
		text.append(floater->getString("no_copy_text"));
	}
	if (!(next_owner_mask & PERM_MODIFY))
	{
		text.append(floater->getString("no_modify_text"));
	}
	if (!(next_owner_mask & PERM_TRANSFER))
	{
		text.append(floater->getString("no_transfer_text"));
	}

	row["columns"][1]["column"] = "text";
	row["columns"][1]["value"] = text;
	row["columns"][1]["font"] = "SANSSERIF";

	// Add after columns added so appropriate heights are correct.
	object_list->addElement(row);

	floater->childSetTextArg("buy_text", "[AMOUNT]", llformat("%d", sale_info.getSalePrice()));
	floater->childSetTextArg("buy_text", "[NAME]", owner_name);

	// Must do this after the floater is created, because
	// sometimes the inventory is already there and 
	// the callback is called immediately.
	LLViewerObject* obj = selection->getFirstRootObject();
	floater->registerVOInventoryListener(obj,NULL);
	floater->requestVOInventory();
}

void LLFloaterBuy::inventoryChanged(LLViewerObject* obj,
								 InventoryObjectList* inv,
								 S32 serial_num,
								 void* data)
{
	if (!obj)
	{
		llwarns << "No object in LLFloaterBuy::inventoryChanged" << llendl;
		return;
	}

	if (!inv)
	{
		llwarns << "No inventory in LLFloaterBuy::inventoryChanged"
			<< llendl;
		removeVOInventoryListener();
		return;
	}

	LLCtrlListInterface *item_list = childGetListInterface("item_list");
	if (!item_list)
	{
		removeVOInventoryListener();
		return;
	}

	InventoryObjectList::const_iterator it = inv->begin();
	InventoryObjectList::const_iterator end = inv->end();
	for ( ; it != end; ++it )
	{
		LLInventoryObject* obj = (LLInventoryObject*)(*it);
			
		// Skip folders, so we know we have inventory items only
		if (obj->getType() == LLAssetType::AT_CATEGORY)
			continue;

		// Skip the mysterious blank InventoryObject 
		if (obj->getType() == LLAssetType::AT_NONE)
			continue;


		LLInventoryItem* inv_item = (LLInventoryItem*)(obj);

		// Skip items we can't transfer
		if (!inv_item->getPermissions().allowTransferTo(gAgent.getID())) 
			continue;

		// Create the line in the list
		LLSD row;

		// Compute icon for this item
		BOOL item_is_multi = FALSE;
		if ( inv_item->getFlags() & LLInventoryItem::II_FLAGS_LANDMARK_VISITED 
			|| inv_item->getFlags() & LLInventoryItem::II_FLAGS_OBJECT_HAS_MULTIPLE_ITEMS)
		{
			item_is_multi = TRUE;
		}

		std::string icon_name = get_item_icon_name(inv_item->getType(), 
							 inv_item->getInventoryType(),
							 inv_item->getFlags(),
							 item_is_multi);
		row["columns"][0]["column"] = "icon";
		row["columns"][0]["type"] = "icon";
		row["columns"][0]["value"] = icon_name;
				
		// Append the permissions that you will acquire (not the current
		// permissions).
		U32 next_owner_mask = inv_item->getPermissions().getMaskNextOwner();
		std::string text = obj->getName();
		if (!(next_owner_mask & PERM_COPY))
		{
			text.append(LLTrans::getString("no_copy"));
		}
		if (!(next_owner_mask & PERM_MODIFY))
		{
			text.append(LLTrans::getString("no_modify"));
		}
		if (!(next_owner_mask & PERM_TRANSFER))
		{
			text.append(LLTrans::getString("no_transfer"));
		}

		row["columns"][1]["column"] = "text";
		row["columns"][1]["value"] = text;
		row["columns"][1]["font"] = "SANSSERIF";

		item_list->addElement(row);
	}
	removeVOInventoryListener();
}

void LLFloaterBuy::onClickBuy()
{
	// Put the items where we put new folders.
	LLUUID category_id;
	category_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_OBJECT);

	// *NOTE: doesn't work for multiple object buy, which UI does not
	// currently support sale info is used for verification only, if
	// it doesn't match region info then sale is canceled.
	LLSelectMgr::getInstance()->sendBuy(gAgent.getID(), category_id, mSaleInfo );

	closeFloater();
}


void LLFloaterBuy::onClickCancel()
{
	closeFloater();
}

// virtual
void LLFloaterBuy::onClose(bool app_quitting)
{
	mObjectSelection.clear();
}
