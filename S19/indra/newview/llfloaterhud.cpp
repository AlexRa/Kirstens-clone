/*
*  Copyright (c) 2008-2009,Kirstenlee Cinquetti (Lee Quick)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Kirstenlee nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Kirstenlee Cinquetti ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Kirstenlee Cinquetti BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
 

#include "llviewerprecompiledheaders.h"

#include "llavatarconstants.h"

//Lots of includes for later useage in some cases
#include "llfloaterhud.h"   // the linux compiler is REALLY pedantic about Upper and lower case Lee!
#include "llviewerwindow.h"
#include "lltracker.h"
#include "llimview.h"
#include "message.h"
#include "llagent.h"
#include "llfloateravatarinfo.h"
#include "llfloaterworldmap.h"
#include "llvoavatar.h"
#include "llfloateractivespeakers.h"
#include "llbutton.h"
#include "llviewermessage.h"
#include "llfocusmgr.h"
#include "llinventoryview.h"
#include "llinventorymodel.h"
#include "lllineeditor.h"
#include "llscrolllistctrl.h"
#include "lltextbox.h"
#include "lluictrlfactory.h"
#include "llagent.h"
#include "llcommandhandler.h"
#include "llfloateravatarpicker.h"
#include "llnamelistctrl.h"
#include "llnotify.h"
#include "llresmgr.h"
#include "llmenucommands.h"
#include "llviewercontrol.h"
#include "lltimer.h"


const S32 MIN_WIDTH = 200;
const S32 MIN_HEIGHT = 340;
const LLRect FLOATER_RECT(0, 380, 240, 0);
const std::string FLOATER_TITLE = "Scanner";
BOOL toggle = FALSE;



// static
LLFloaterHUD* LLFloaterHUD::sInstance = NULL;


LLFloaterHUD* LLFloaterHUD::show(callback_t callback,void* userdata,BOOL allow_multiple,BOOL closeOnSelect)
{
	if (!sInstance)
	{
		sInstance = new LLFloaterHUD();
		sInstance->mCallback = callback;
		sInstance->mCallbackUserdata = userdata;
		sInstance->mCloseOnSelect = FALSE;
        sInstance->mHalt = FALSE;
		sInstance->open();	/* Flawfinder: ignore */
		sInstance->center();
		
		
	}
	else
	{
		sInstance->open();	/*Flawfinder: ignore*/
		sInstance->mCallback = callback;
		sInstance->mCallbackUserdata = userdata;
				
	}
	
	sInstance->mCloseOnSelect = closeOnSelect;
	return sInstance;
}

// Default constructor
LLFloaterHUD::LLFloaterHUD() :
	LLFloater(std::string("scanner"), FLOATER_RECT, FLOATER_TITLE, TRUE, MIN_WIDTH, MIN_HEIGHT),
	mResultsReturned(FALSE),
	mCallback(NULL),
	mCallbackUserdata(NULL)

{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_avatar_scanner.xml", NULL);
}

BOOL LLFloaterHUD::postBuild()
{
	
	
	childSetAction("Find ON", onBtnFind, this); // the toggle
	

	mListNames = getChild<LLScrollListCtrl>("Names"); // double clicking a name in the list does action
	childSetDoubleClickCallback("Names",onBtnAdd);  // punches up the profile of the selected avie on the list.
	childSetCommitCallback("Names", onList, this);
	// childDisable("Names");

    childSetAction("Select", onBtnLocate, this);  // punches up the profile of the selected avie on the list.
	// childDisable("Select");

	childSetAction("Close", onBtnClose, this);

	
	return TRUE;
}

// Destroys the object
LLFloaterHUD::~LLFloaterHUD()
{
	gFocusMgr.releaseFocusIfNeeded( this );

	sInstance = NULL;
}

void LLFloaterHUD::updater()
{
		
    LLFloaterHUD* self = sInstance;
	if(self) self->find();
	

}

void LLFloaterHUD::onBtnFind(void* userdata) // Pressing the scan button toggles the scanner.
{
	
	LLFloaterHUD* self = (LLFloaterHUD*)userdata;
	if(self) 
	{
	if(sInstance->mHalt == TRUE)
	{
		sInstance->mHalt = FALSE;
		toggle = FALSE;
		
	}
	else if(sInstance->mHalt == FALSE)
	{
		sInstance->mHalt = TRUE;
		toggle = TRUE;
				
	}
	}
	
	
  return;
	
}

void LLFloaterHUD::onBtnAdd(void* userdata)  //Punch up a profile from the list!
{
  
  LLFloaterHUD* self = (LLFloaterHUD*)userdata;
  LLScrollListItem *item = self->mListNames->getFirstSelected();

  if ( NULL == item ) return; //NULLS are BAD!! lets not continue and just return ^^

  LLUUID agent_id = item->getUUID();
  llinfos << "scanner gets the profile kirsten" << llendl;
  { LLFloaterAvatarInfo::showFromDirectory(agent_id); }
}

void LLFloaterHUD::onBtnLocate(void* userdata)  //some disabled code does not work yet.... for future ref :)
{
  
        LLFloaterHUD* self = (LLFloaterHUD*)userdata;
        LLScrollListItem *item = self->mListNames->getFirstSelected();

		if ( NULL == item ) return;
				
        LLUUID agent_id = item->getUUID();

		handle_lure(agent_id); //Offering a TP does work client side , not working server side without presence!
				
		/*
		LLVOAvatar* avatarp = (LLVOAvatar*)item;
		std::string tooltip("");
	    std::string name("");
			    
		LLVector3d position = gAgent.getPosGlobalFromAgent(avatarp->getCharacterPosition());
		LLVector3d mypos = gAgent.getPositionGlobal();
        LLVector3d delta =  mypos - position;

       
		LLTracker::trackAvatar(agent_id, name);
		llinfos << "delta = " << delta << "info Kirsten" << llendl;
		llinfos << "position = " << position << "info Kirsten" << llendl;
		LLTracker::trackLocation(position, name, tooltip);
	    gAgent.teleportViaLocation( position); */
	
	
}

void LLFloaterHUD::onBtnClose(void* userdata)  //kill the scanner close button.
{
	LLFloaterHUD* self = (LLFloaterHUD*)userdata;
	if(self) self->close();

}


void LLFloaterHUD::onList(LLUICtrl* ctrl, void* userdata)
{
	LLFloaterHUD* self = (LLFloaterHUD*)userdata;
	if (!self)
	{
		return;
	}

	self->mAvatarIDs.clear();
	self->mAvatarNames.clear();

	if (!self->mListNames)
	{
		return;
	}
	
	std::vector<LLScrollListItem*> items =
		self->mListNames->getAllSelected();
	for (
		std::vector<LLScrollListItem*>::iterator iter = items.begin();
		iter != items.end();
		++iter)
	{
		LLScrollListItem* item = *iter;
		self->mAvatarNames.push_back(item->getColumn(0)->getValue().asString());
		self->mAvatarIDs.push_back(item->getUUID());
		self->childSetEnabled("Select", TRUE);
	}
}


void LLFloaterHUD::find() // Scanner activated by Scan button
{
	if(mHalt) return; // this basically checks if the scan has been toggled on or off otherwise llvoavatar calls the scan on avatar activity
	

	mListNames->deleteAllItems(); //Clear the list

		
	std::vector< LLCharacter* >::iterator avatar_it;
	for(avatar_it = LLCharacter::sInstances.begin(); avatar_it != LLCharacter::sInstances.end(); ++avatar_it)
	{
		LLVOAvatar* avatarp = (LLVOAvatar*)*avatar_it;

		if (avatarp->isDead() || avatarp->isSelf()) //Dont show the user!
		{
			continue;
		} 

		if (dist_vec(avatarp->getPositionAgent(), gAgent.getPositionAgent()) <= SCAN_MAX_RADIUS) //Scanner radius set in indra constants.
		{
			
			// Pull in that avatar data!
			std::string name = avatarp->getFullname();
			LLVector3d position = gAgent.getPosGlobalFromAgent(avatarp->getCharacterPosition());
		    LLUUID avid = avatarp->getID();

		    // Work out distance relative to user!
			LLVector3d mypos = gAgent.getPositionGlobal();
            LLVector3d delta = position - mypos;
		    F32 distance = (F32)delta.magVec();

            //Build the list
			LLSD element;

            element["id"] = avid;

			element["columns"][LIST_AVATAR_NAME]["column"] = "name";
			element["columns"][LIST_AVATAR_NAME]["value"] = name;

            element["columns"][LIST_DISTANCE]["column"] = "distance";
			element["columns"][LIST_DISTANCE]["value"] = distance;

			
            mListNames->addElement(element);
		 
            mListNames->sortByColumn("distance", TRUE);
			mListNames->setCallbackUserData(this);
			

		}

	}
   return;	
}



void LLFloaterHUD::setAllowMultiple(BOOL allow_multiple)
{
	mAllowMultiple = allow_multiple;
	if (mInventoryPanel)
	{
		mInventoryPanel->setAllowMultiSelect(mAllowMultiple);
	}
	if (mListNames)
	{
		mListNames->setAllowMultipleSelection(mAllowMultiple);
	}
}

// static 
void LLFloaterHUD::processHUDReply(LLMessageSystem* msg, void**)
{
	LLUUID	agent_id;
	LLUUID	query_id;
	LLUUID	avatar_id;
	std::string first_name;
	std::string last_name;

	msg->getUUID("AgentData", "AgentID", agent_id);
	msg->getUUID("AgentData", "QueryID", query_id);

	// Not for us
	if (agent_id != gAgent.getID()) return;

	// Dialog already closed
	LLFloaterHUD *self = sInstance;
	if (!self) return;

	// these are not results from our last request
	if (query_id != self->mQueryID)
	{
		return;
	}

	if (!self->mResultsReturned)
	{
		// clear "Searching" label on first results
		if (self->mListNames)
		{
			self->mListNames->deleteAllItems();
		}
	}
	self->mResultsReturned = TRUE;

	if (self->mListNames)
	{
		BOOL found_one = FALSE;
		S32 num_new_rows = msg->getNumberOfBlocks("Data");
		for (S32 i = 0; i < num_new_rows; i++)
		{			
			msg->getUUIDFast(  _PREHASH_Data,_PREHASH_AvatarID,	avatar_id, i);
			msg->getStringFast(_PREHASH_Data,_PREHASH_FirstName, first_name, i);
			msg->getStringFast(_PREHASH_Data,_PREHASH_LastName,	last_name, i);
		
			std::string avatar_name;
			if (avatar_id.isNull())
			{
				LLStringUtil::format_map_t map;
				map["[TEXT]"] = self->childGetText("Edit");
				avatar_name = self->getString("NotFound", map);
				self->mListNames->setEnabled(FALSE);
			}
			else
			{
				avatar_name = first_name + " " + last_name;
				self->mListNames->setEnabled(TRUE);
				found_one = TRUE;
			}
			LLSD element;
			element["id"] = avatar_id; // value
			element["columns"][0]["value"] = avatar_name;
			self->mListNames->addElement(element);
		}
	
		if (found_one)
		{
			self->mListNames->selectFirstItem();
			self->onList(self->mListNames, self);
			self->mListNames->setFocus(TRUE);
		}
	}
}


// virtual
BOOL LLFloaterHUD::handleKeyHere(KEY key, MASK mask)
{
	if (key == KEY_RETURN
		&& mask == MASK_NONE)
	{
		if (childHasFocus("Edit"))
		{
			onBtnFind(this);
			return TRUE;
		}
		else
		{
			onBtnAdd(this);
			return TRUE;
		}
	}
	else if (key == KEY_ESCAPE && mask == MASK_NONE)
	{
		close();
		return TRUE;
	}

	return LLFloater::handleKeyHere(key, mask);
}
