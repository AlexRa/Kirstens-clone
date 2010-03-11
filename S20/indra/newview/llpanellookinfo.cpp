/**
 * @file llpanellookinfo.cpp
 * @brief Displays place information in Side Tray.
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009-2010, Linden Research, Inc.
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

#include "llpanellookinfo.h"

// *TODO: reorder includes to match the coding standard
#include "llagent.h"
#include "llagentwearables.h"
#include "llappearancemgr.h"
#include "llinventory.h"
#include "llviewercontrol.h"
#include "llui.h"
#include "llfloater.h"
#include "llfloaterreg.h"
#include "llinventoryfunctions.h"
#include "llinventorypanel.h"
#include "llviewerwindow.h"
#include "llviewerinventory.h"
#include "llbutton.h"
#include "llcombobox.h"
#include "llfiltereditor.h"
#include "llfloaterinventory.h"
#include "llinventorybridge.h"
#include "llinventorymodel.h"
#include "lluiconstants.h"
#include "llscrolllistctrl.h"
#include "lltextbox.h"
#include "lluictrlfactory.h"
#include "llsdutil.h"
#include "llsidepanelappearance.h"
#include "llwearablelist.h"

static LLRegisterPanelClassWrapper<LLPanelLookInfo> t_look("panel_look_info");

const U64 WEARABLE_MASK = (1LL << LLInventoryType::IT_WEARABLE);
const U64 ATTACHMENT_MASK = (1LL << LLInventoryType::IT_ATTACHMENT) | (1LL << LLInventoryType::IT_OBJECT);
const U64 ALL_ITEMS_MASK = WEARABLE_MASK | ATTACHMENT_MASK;

class LLInventoryLookObserver : public LLInventoryObserver
{
public:
	LLInventoryLookObserver(LLPanelLookInfo *panel) : mPanel(panel) {}
	virtual ~LLInventoryLookObserver() 
	{
		if (gInventory.containsObserver(this))
		{
			gInventory.removeObserver(this);
		}
	}
	
	virtual void changed(U32 mask)
	{
		if (mask & (LLInventoryObserver::ADD | LLInventoryObserver::REMOVE))
		{
			mPanel->updateLookInfo();
		}
	}
protected:
	LLPanelLookInfo *mPanel;
};

class LLLookFetchObserver : public LLInventoryFetchDescendentsObserver
{
public:
	LLLookFetchObserver(LLPanelLookInfo *panel) :
	mPanel(panel)
	{}
	LLLookFetchObserver() {}
	virtual void done()
	{
		mPanel->lookFetched();
		if(gInventory.containsObserver(this))
		{
			gInventory.removeObserver(this);
		}
	}
private:
	LLPanelLookInfo *mPanel;
};



LLPanelLookInfo::LLPanelLookInfo()
:	LLPanel(), mLookID(), mFetchLook(NULL), mSearchFilter(NULL),
mLookContents(NULL), mInventoryItemsPanel(NULL), mAddToLookBtn(NULL),
mRemoveFromLookBtn(NULL), mLookObserver(NULL), mNumItemsInLook(0)
{
	mSavedFolderState = new LLSaveFolderState();
	mSavedFolderState->setApply(FALSE);
	
	mFetchLook = new LLLookFetchObserver(this);
	mLookObserver = new LLInventoryLookObserver(this);
	gInventory.addObserver(mLookObserver);
	
	mLookItemTypes.reserve(NUM_LOOK_ITEM_TYPES);
	for (U32 i = 0; i < NUM_LOOK_ITEM_TYPES; i++)
	{
		mLookItemTypes.push_back(LLLookItemType());
	}
	
	// TODO: make these strings translatable
	mLookItemTypes[LIT_ALL] = LLLookItemType("All Items", ALL_ITEMS_MASK);
	mLookItemTypes[LIT_WEARABLE] = LLLookItemType("Shape & Clothing", WEARABLE_MASK);
	mLookItemTypes[LIT_ATTACHMENT] = LLLookItemType("Attachments", ATTACHMENT_MASK);

}

LLPanelLookInfo::~LLPanelLookInfo()
{
	delete mSavedFolderState;
	if (gInventory.containsObserver(mFetchLook))
	{
		gInventory.removeObserver(mFetchLook);
	}
	delete mFetchLook;
	
	if (gInventory.containsObserver(mLookObserver))
	{
		gInventory.removeObserver(mLookObserver);
	}
	delete mLookObserver;
}

BOOL LLPanelLookInfo::postBuild()
{
	// gInventory.isInventoryUsable() no longer needs to be tested per Richard's fix for race conditions between inventory and panels
		
	mLookName = getChild<LLTextBox>("curr_look_name"); 
	
	/*
	mLookContents->setDoubleClickCallback(onDoubleClickSpeaker, this);
	mLookContents->setCommitOnSelectionChange(TRUE);
	mLookContents->setCommitCallback(boost::bind(&LLPanelActiveSpeakers::handleSpeakerSelect, this, _2));
	mLookContents->setSortChangedCallback(boost::bind(&LLPanelActiveSpeakers::onSortChanged, this));
	mLookContents->setContextMenu(LLScrollListCtrl::MENU_AVATAR);
	*/
	
	mInventoryItemsPanel = getChild<LLInventoryPanel>("inventory_items");
	mInventoryItemsPanel->setFilterTypes(ALL_ITEMS_MASK);
	mInventoryItemsPanel->setShowFolderState(LLInventoryFilter::SHOW_NON_EMPTY_FOLDERS);
	// mInventoryItemsPanel->setSelectCallback(boost::bind(&LLPanelLookInfo::onInventorySelectionChange, this, _1, _2));
	// mInventoryItemsPanel->getRootFolder()->setReshapeCallback(boost::bind(&LLPanelLookInfo::onInventorySelectionChange, this, _1, _2));
	
	LLComboBox* type_filter = getChild<LLComboBox>("inventory_filter");
	type_filter->setCommitCallback(boost::bind(&LLPanelLookInfo::onTypeFilterChanged, this, _1));
	type_filter->removeall();
	for (U32 i = 0; i < mLookItemTypes.size(); ++i)
	{
		type_filter->add(mLookItemTypes[i].displayName);
	}
	type_filter->setCurrentByIndex(LIT_ALL);
	
	mSearchFilter = getChild<LLFilterEditor>("look_item_filter");
	mSearchFilter->setCommitCallback(boost::bind(&LLPanelLookInfo::onSearchEdit, this, _2));
	
	/* Removing add to look inline button (not part of mvp for viewer 2)
	LLButton::Params add_params;
	add_params.name("add_to_look");
	add_params.click_callback.function(boost::bind(&LLPanelLookInfo::onAddToLookClicked, this));
	add_params.label("+");
	
	mAddToLookBtn = LLUICtrlFactory::create<LLButton>(add_params);
	mAddToLookBtn->setEnabled(FALSE);
	mAddToLookBtn->setVisible(FALSE); */
	
	childSetAction("add_item_btn", boost::bind(&LLPanelLookInfo::onAddToLookClicked, this), this);

	mUpBtn = getChild<LLButton>("up_btn");
	mUpBtn->setEnabled(TRUE);
	mUpBtn->setClickedCallback(boost::bind(&LLPanelLookInfo::onUpClicked, this));
	
	mLookContents = getChild<LLScrollListCtrl>("look_items_list");
	mLookContents->sortByColumn("look_item_sort", TRUE);
	mLookContents->setCommitCallback(boost::bind(&LLPanelLookInfo::onLookItemSelectionChange, this));
	
	/*
	LLButton::Params remove_params;
	remove_params.name("remove_from_look");
	remove_params.click_callback.function(boost::bind(&LLPanelLookInfo::onRemoveFromLookClicked, this));
	remove_params.label("-"); */
	
	//mRemoveFromLookBtn = LLUICtrlFactory::create<LLButton>(remove_params);
	mRemoveFromLookBtn = getChild<LLButton>("remove_from_look_btn");
	mRemoveFromLookBtn->setEnabled(FALSE);
	mRemoveFromLookBtn->setVisible(FALSE);
	//childSetAction("remove_from_look_btn", boost::bind(&LLPanelLookInfo::onRemoveFromLookClicked, this), this);
	mRemoveFromLookBtn->setCommitCallback(boost::bind(&LLPanelLookInfo::onRemoveFromLookClicked, this));
	//getChild<LLPanel>("look_info_group_bar")->addChild(mRemoveFromLookBtn); remove_item_btn
	
	childSetAction("remove_item_btn", boost::bind(&LLPanelLookInfo::onRemoveFromLookClicked, this), this);
	
	return TRUE;
}

void LLPanelLookInfo::onTypeFilterChanged(LLUICtrl* ctrl)
{
	LLComboBox* type_filter = dynamic_cast<LLComboBox*>(ctrl);
	llassert(type_filter);
	if (type_filter)
	{
		U32 curr_filter_type = type_filter->getCurrentIndex();
		mInventoryItemsPanel->setFilterTypes(mLookItemTypes[curr_filter_type].inventoryMask);
	}
	
	mSavedFolderState->setApply(TRUE);
	mInventoryItemsPanel->getRootFolder()->applyFunctorRecursively(*mSavedFolderState);
	
	LLOpenFoldersWithSelection opener;
	mInventoryItemsPanel->getRootFolder()->applyFunctorRecursively(opener);
	mInventoryItemsPanel->getRootFolder()->scrollToShowSelection();
	
	gInventory.startBackgroundFetch();
}

void LLPanelLookInfo::onSearchEdit(const std::string& string)
{
	if (mSearchString != string)
	{
		mSearchString = string;
		
		// Searches are case-insensitive
		LLStringUtil::toUpper(mSearchString);
		LLStringUtil::trimHead(mSearchString);
	}
	
	if (mSearchString == "")
	{
		mInventoryItemsPanel->setFilterSubString(LLStringUtil::null);
		
		// re-open folders that were initially open
		mSavedFolderState->setApply(TRUE);
		mInventoryItemsPanel->getRootFolder()->applyFunctorRecursively(*mSavedFolderState);
		LLOpenFoldersWithSelection opener;
		mInventoryItemsPanel->getRootFolder()->applyFunctorRecursively(opener);
		mInventoryItemsPanel->getRootFolder()->scrollToShowSelection();
	}
	
	gInventory.startBackgroundFetch();
	
	if (mInventoryItemsPanel->getFilterSubString().empty() && mSearchString.empty())
	{
		// current filter and new filter empty, do nothing
		return;
	}
	
	// save current folder open state if no filter currently applied
	if (mInventoryItemsPanel->getRootFolder()->getFilterSubString().empty())
	{
		mSavedFolderState->setApply(FALSE);
		mInventoryItemsPanel->getRootFolder()->applyFunctorRecursively(*mSavedFolderState);
	}
	
	// set new filter string
	mInventoryItemsPanel->setFilterSubString(mSearchString);
}

void LLPanelLookInfo::onAddToLookClicked(void)
{
	LLFolderViewItem* curr_item = mInventoryItemsPanel->getRootFolder()->getCurSelectedItem();
	LLFolderViewEventListener* listenerp  = curr_item->getListener();
	link_inventory_item(gAgent.getID(), listenerp->getUUID(), mLookID, listenerp->getName(),
						LLAssetType::AT_LINK, LLPointer<LLInventoryCallback>(NULL));
	updateLookInfo();
}


void LLPanelLookInfo::onRemoveFromLookClicked(void)
{
	LLUUID id_to_remove = mLookContents->getSelectionInterface()->getCurrentID();
	
	LLViewerInventoryItem * item_to_remove = gInventory.getItem(id_to_remove);
	
	if (item_to_remove)
	{
		// returns null if not a wearable (attachment, etc).
		const LLWearable* wearable_to_remove = gAgentWearables.getWearableFromAssetID(item_to_remove->getAssetUUID());
		if (!wearable_to_remove || gAgentWearables.canWearableBeRemoved( wearable_to_remove ))
		{											 
			gInventory.purgeObject( id_to_remove );
			updateLookInfo();
			mRemoveFromLookBtn->setEnabled(FALSE);
			if (mRemoveFromLookBtn->getVisible())
			{
				mRemoveFromLookBtn->setVisible(FALSE);
			}
		}
	}
}


void LLPanelLookInfo::onUpClicked(void)
{
	LLUUID inv_id = mLookContents->getSelectionInterface()->getCurrentID();
	if (inv_id.isNull())
	{
		//nothing selected, do nothing
		return;
	}

	LLViewerInventoryItem *link_item = gInventory.getItem(inv_id);
	if (!link_item)
	{
		llwarns << "could not find inventory item based on currently worn link." << llendl;
		return;
	}


	LLUUID asset_id = link_item->getAssetUUID();
	if (asset_id.isNull())
	{
		llwarns << "inventory link has null Asset ID. could not get object reference" << llendl;
	}

	static const std::string empty = "";
	LLWearableList::instance().getAsset(asset_id,
										empty,	// don't care about wearable name
										link_item->getActualType(),
										LLSidepanelAppearance::editWearable,
										(void*)getParentUICtrl());
}

void LLPanelLookInfo::onInventorySelectionChange(const std::deque<LLFolderViewItem*> &items, BOOL user_action)
{
	LLFolderViewItem* current_item = mInventoryItemsPanel->getRootFolder()->getCurSelectedItem();
	if (!current_item)
	{
		return;
	}
	
	/* Removing add to look inline button (not part of mvp for viewer 2)
	LLRect btn_rect(current_item->getLocalRect().mRight - 50,
					current_item->getLocalRect().mTop,
					current_item->getLocalRect().mRight - 30,
					current_item->getLocalRect().mBottom);
	
	mAddToLookBtn->setRect(btn_rect);
	mAddToLookBtn->setEnabled(TRUE);
	if (!mAddToLookBtn->getVisible())
	{
		mAddToLookBtn->setVisible(TRUE);
	}
	
	current_item->addChild(mAddToLookBtn); */
}

void LLPanelLookInfo::onLookItemSelectionChange(void)
{	
	S32 left_offset = -4;
	S32 top_offset = -10;
	LLRect rect = mLookContents->getLastSelectedItem()->getRect();
	LLRect btn_rect(
					left_offset + rect.mRight - 50,
					top_offset  + rect.mTop,
					left_offset + rect.mRight - 30,
					top_offset  + rect.mBottom);
	
	mRemoveFromLookBtn->setRect(btn_rect);
	
	mRemoveFromLookBtn->setEnabled(TRUE);
	if (!mRemoveFromLookBtn->getVisible())
	{
		mRemoveFromLookBtn->setVisible(TRUE);
	}
	//mLookContents->addChild(mRemoveFromLookBtn);
}

void LLPanelLookInfo::changed(U32 mask)
{
}

void LLPanelLookInfo::lookFetched(void)
{
	LLInventoryModel::cat_array_t cat_array;
	LLInventoryModel::item_array_t item_array;

	// collectDescendentsIf takes non-const reference:
	LLFindCOFValidItems is_cof_valid;
	gInventory.collectDescendentsIf(mLookID,
									cat_array,
									item_array,
									LLInventoryModel::EXCLUDE_TRASH,
									is_cof_valid);
	for (LLInventoryModel::item_array_t::const_iterator iter = item_array.begin();
		 iter != item_array.end();
		 iter++)
	{
		const LLViewerInventoryItem *item = (*iter);
		
		LLSD row;
		row["id"] = item->getUUID();
		LLSD& columns = row["columns"];
		columns[0]["column"] = "look_item";
		columns[0]["type"] = "text";
		columns[0]["value"] = item->getName();
		columns[1]["column"] = "look_item_sort";
		columns[1]["type"] = "text"; // TODO: multi-wearable sort "type" should go here.
		columns[1]["value"] = "BAR"; // TODO: Multi-wearable sort index should go here
		
		mLookContents->addElement(row);
	}
	
	if (mLookContents->getItemCount() != mNumItemsInLook)
	{
		mNumItemsInLook = mLookContents->getItemCount();
		LLAppearanceManager::instance().updateCOF(mLookID);
	}
}

void LLPanelLookInfo::updateLookInfo()
{	
	if (getVisible())
	{
		mLookContents->clearRows();
		
		LLInventoryFetchDescendentsObserver::folder_ref_t folders;
		folders.push_back(mLookID);
		mFetchLook->fetchDescendents(folders);
		if (mFetchLook->isEverythingComplete())
		{
			mFetchLook->done();
		}
		else
		{
			gInventory.addObserver(mFetchLook);
		}
	}
}

void LLPanelLookInfo::displayLookInfo(const LLInventoryCategory* pLook)
{
	if (!pLook)
	{
		return;
	}
	
	if (!getVisible())
	{
		setVisible(TRUE);
	}

	if (mLookID != pLook->getUUID())
	{
		mLookID = pLook->getUUID();
		mLookName->setText("Look: " + pLook->getName());
		updateLookInfo();
	}
}

void LLPanelLookInfo::reset()
{
	mLookID.setNull();
}

