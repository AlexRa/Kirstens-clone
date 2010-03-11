/** 
 * @file llpanelplace.h
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

#ifndef LL_LLPANELLOOKINFO_H
#define LL_LLPANELLOOKINFO_H

#include "llpanel.h"

#include "v3dmath.h"
#include "lluuid.h"

#include "lliconctrl.h"

#include "llremoteparcelrequest.h"
#include "llinventory.h"
#include "llinventorymodel.h"

class LLButton;
class LLTextBox;
class LLInventoryCategory;
class LLInventoryLookObserver;
class LLInventoryPanel;
class LLSaveFolderState;
class LLFolderViewItem;
class LLScrollListCtrl;
class LLLookFetchObserver;
class LLFilterEditor;

class LLPanelLookInfo : public LLPanel
{
public:
	
	// NOTE: initialize mLookItemTypes at the index of any new enum you add in the LLPanelLookInfo() constructor
	typedef enum e_look_item_type
	{
		LIT_ALL = 0,
		LIT_WEARABLE, // clothing or shape
		LIT_ATTACHMENT,
		NUM_LOOK_ITEM_TYPES
	} ELookItemType; 
	
	struct LLLookItemType {
		std::string displayName;
		U64 inventoryMask;
		LLLookItemType() : displayName("NONE"), inventoryMask(0) {}
		LLLookItemType(std::string name, U64 mask) : displayName(name), inventoryMask(mask) {}
	};
	
	LLPanelLookInfo();
	/*virtual*/ ~LLPanelLookInfo();

	/*virtual*/ BOOL postBuild();
	/*virtual*/ void changed(U32 mask);

	void reset();
		// Ignore all old information, useful if you are 
		// recycling an existing dialog and need to clear it.

	/*virtual*/ void setParcelID(const LLUUID& parcel_id);
		// Sends a request for data about the given parcel, which will
		// only update the location if there is none already available.
	
	void onTypeFilterChanged(LLUICtrl* ctrl);
	void onSearchEdit(const std::string& string);
	void onInventorySelectionChange(const std::deque<LLFolderViewItem*> &items, BOOL user_action);
	void onAddToLookClicked(void);
	void onLookItemSelectionChange(void);
	void onRemoveFromLookClicked(void);
	void onUpClicked(void);

	void displayLookInfo(const LLInventoryCategory* pLook);
	
	void lookFetched(void);
	
	void updateLookInfo(void);

private:

	LLUUID				mLookID;
	LLTextBox*			mLookName;
	LLScrollListCtrl*	mLookContents;
	LLInventoryPanel*	mInventoryItemsPanel;
	LLFilterEditor*		mSearchFilter;
	LLSaveFolderState*	mSavedFolderState;
	std::string			mSearchString;
	LLButton*			mAddToLookBtn;
	LLButton*			mRemoveFromLookBtn;
	LLButton*			mUpBtn;
	S32					mNumItemsInLook;
	
	LLLookFetchObserver*		mFetchLook;
	LLInventoryLookObserver*	mLookObserver;
	std::vector<LLLookItemType> mLookItemTypes;
};

#endif // LL_LLPANELLOOKINFO_H
