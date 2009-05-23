/*
*  Copyright (c) 2008-2009,Kirstenlee Openlife (Lee Quick)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Openlife / 3DX nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Kirstenlee Openlife ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Kirstenlee Openlife BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LL_LLFLOATERHUD_H
#define LL_LLFLOATERHUD_H

#include "llfloater.h"
#include "lltimer.h"

#include <vector>

class LLUICtrl;
class LLTextBox;
class LLLineEditor;
class LLButton;
class LLScrollListCtrl;
class LLMessageSystem;
class LLInventoryPanel;
class LLFolderViewItem;

// avatar picker header modified for the purpose!

class LLFloaterHUD : public LLFloater
{
public:
	// Call this to select an avatar.
	// The callback function will be called with an avatar name and UUID.
	typedef void(*callback_t)(const std::vector<std::string>&, const std::vector<LLUUID>&, void*);
   static LLFloaterHUD* show(callback_t callback, 
									   void* userdata,
									   BOOL allow_multiple = FALSE,
									   BOOL closeOnSelect = FALSE);   
	
	virtual	BOOL postBuild();
    static void processHUDReply(LLMessageSystem* msg, void**);
	static void editKeystroke(LLLineEditor* caller, void* user_data);
		

    enum AVATARS_COLUMN_ORDER
	{
		LIST_AVATAR_NAME,
		LIST_DISTANCE,
		
	};

public:
	static void* createInventoryPanel(void* userdata);
    static void onBtnLocate(void* userdata);
	static void onBtnFind(void* userdata);
	static void onBtnAdd(void* userdata);
	static void onBtnClose(void* userdata);
	static void onList(LLUICtrl* ctrl, void* userdata);
	
		   void doSelectionChange(const std::deque<LLFolderViewItem*> &items, BOOL user_action, void* data);
	static void onSelectionChange(const std::deque<LLFolderViewItem*> &items, BOOL user_action, void* data);
    
	void populateNearMe();
	void find();
	static void update();
	void setAllowMultiple(BOOL allow_multiple);
	
	virtual BOOL handleKeyHere(KEY key, MASK mask);


	LLScrollListCtrl*	mListNames;
	LLInventoryPanel*	mInventoryPanel;
	
	std::vector<LLUUID>				mAvatarIDs;
	std::vector<std::string>		mAvatarNames;
	BOOL				mAllowMultiple;
	LLUUID				mQueryID;
	BOOL				mResultsReturned;
	BOOL				mCloseOnSelect;

	void (*mCallback)(const std::vector<std::string>& name, const std::vector<LLUUID>& id, void* userdata);
	void* mCallbackUserdata;
	BOOL				mListComplete;
	BOOL                mHalt;


	static LLFloaterHUD* sInstance;

protected:
	// do not call these directly
	LLFloaterHUD();
	virtual ~LLFloaterHUD();
};

#endif // LL_LLFLOATERHUD_H
