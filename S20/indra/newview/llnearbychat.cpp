/** 
 * @file LLNearbyChat.cpp
 * @brief Nearby chat history scrolling panel implementation
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

#include "llnearbychat.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llrootview.h"
//#include "llchatitemscontainerctrl.h"
#include "lliconctrl.h"
#include "llsidetray.h"
#include "llfocusmgr.h"
#include "llresizebar.h"
#include "llresizehandle.h"
#include "llmenugl.h"
#include "llviewermenu.h"//for gMenuHolder

#include "llnearbychathandler.h"
#include "llchannelmanager.h"

#include "llagent.h" 			// gAgent
#include "llfloaterscriptdebug.h"
#include "llchathistory.h"
#include "llstylemap.h"

#include "lldraghandle.h"

#include "llbottomtray.h"
#include "llnearbychatbar.h"
#include "llfloaterreg.h"
#include "lltrans.h"

static const S32 RESIZE_BAR_THICKNESS = 3;

const static std::string IM_TIME("time");
const static std::string IM_TEXT("message");
const static std::string IM_FROM("from");
const static std::string IM_FROM_ID("from_id");


LLNearbyChat::LLNearbyChat(const LLSD& key) 
	: LLDockableFloater(NULL, false, false, key)
	,mChatHistory(NULL)
{
	
}

LLNearbyChat::~LLNearbyChat()
{
}

BOOL LLNearbyChat::postBuild()
{
	//menu
	LLUICtrl::CommitCallbackRegistry::ScopedRegistrar registrar;
	LLUICtrl::EnableCallbackRegistry::ScopedRegistrar enable_registrar;

	enable_registrar.add("NearbyChat.Check", boost::bind(&LLNearbyChat::onNearbyChatCheckContextMenuItem, this, _2));
	registrar.add("NearbyChat.Action", boost::bind(&LLNearbyChat::onNearbyChatContextMenuItemClicked, this, _2));

	
	LLMenuGL* menu = LLUICtrlFactory::getInstance()->createFromFile<LLMenuGL>("menu_nearby_chat.xml", gMenuHolder, LLViewerMenuHolderGL::child_registry_t::instance());
	if(menu)
		mPopupMenuHandle = menu->getHandle();

	gSavedSettings.declareS32("nearbychat_showicons_and_names",2,"NearByChat header settings",true);

	mChatHistory = getChild<LLChatHistory>("chat_history");

	if(!LLDockableFloater::postBuild())
		return false;

	if (getDockControl() == NULL)
	{
		setDockControl(new LLDockControl(
			LLBottomTray::getInstance()->getNearbyChatBar(), this,
			getDockTongue(), LLDockControl::TOP, boost::bind(&LLNearbyChat::getAllowedRect, this, _1)));
	}

	setIsChrome(true);
	//chrome="true" hides floater caption 
	if (mDragHandle)
		mDragHandle->setTitleVisible(TRUE);

	return true;
}


void    LLNearbyChat::applySavedVariables()
{
	if (mRectControl.size() > 1)
	{
		const LLRect& rect = LLUI::sSettingGroups["floater"]->getRect(mRectControl);
		if(!rect.isEmpty() && rect.isValid())
		{
			reshape(rect.getWidth(), rect.getHeight());
			setRect(rect);
		}
	}


	if(!LLUI::sSettingGroups["floater"]->controlExists(mDocStateControl))
	{
		setDocked(true);
	}
	else
	{
		if (mDocStateControl.size() > 1)
		{
			bool dockState = LLUI::sSettingGroups["floater"]->getBOOL(mDocStateControl);
			setDocked(dockState);
		}
	}
}

std::string appendTime()
{
	time_t utc_time;
	utc_time = time_corrected();
	std::string timeStr ="["+ LLTrans::getString("TimeHour")+"]:["
		+LLTrans::getString("TimeMin")+"]";

	LLSD substitution;

	substitution["datetime"] = (S32) utc_time;
	LLStringUtil::format (timeStr, substitution);

	return timeStr;
}


void	LLNearbyChat::addMessage(const LLChat& chat,bool archive,const LLSD &args)
{
	if (chat.mChatType == CHAT_TYPE_DEBUG_MSG)
	{
		if(gSavedSettings.getBOOL("ShowScriptErrors") == FALSE)
			return;
		if (gSavedSettings.getS32("ShowScriptErrorsLocation")== 1)// show error in window //("ScriptErrorsAsChat"))
		{

			LLColor4 txt_color;

			LLViewerChat::getChatColor(chat,txt_color);
			
			LLFloaterScriptDebug::addScriptLine(chat.mText,
												chat.mFromName, 
												txt_color, 
												chat.mFromID);
			return;
		}
	}

	LLChat& tmp_chat = const_cast<LLChat&>(chat);

	if(tmp_chat.mTimeStr.empty())
		tmp_chat.mTimeStr = appendTime();

	bool use_plain_text_chat_history = gSavedSettings.getBOOL("PlainTextChatHistory");
	
	if (!chat.mMuted)
	{
		tmp_chat.mFromName = chat.mFromName;
		LLSD chat_args = args;
		chat_args["use_plain_text_chat_history"] = use_plain_text_chat_history;
		mChatHistory->appendMessage(chat, chat_args);
	}

	if(archive)
	{
		mMessageArchive.push_back(chat);
		if(mMessageArchive.size()>200)
			mMessageArchive.erase(mMessageArchive.begin());
	}

	if (args["do_not_log"].asBoolean()) 
	{
		return;
	}

	if (gSavedPerAccountSettings.getBOOL("LogChat")) 
	{
		LLLogChat::saveHistory("chat", chat.mFromName, chat.mFromID, chat.mText);
	}
}

void LLNearbyChat::onNearbySpeakers()
{
	LLSD param;
	param["people_panel_tab_name"] = "nearby_panel";
	LLSideTray::getInstance()->showPanel("panel_people",param);
}


void	LLNearbyChat::onNearbyChatContextMenuItemClicked(const LLSD& userdata)
{
}
bool	LLNearbyChat::onNearbyChatCheckContextMenuItem(const LLSD& userdata)
{
	std::string str = userdata.asString();
	if(str == "nearby_people")
		onNearbySpeakers();	
	return false;
}

void	LLNearbyChat::setVisible(BOOL visible)
{
	if(visible)
	{
		LLNotificationsUI::LLScreenChannelBase* chat_channel = LLNotificationsUI::LLChannelManager::getInstance()->findChannelByID(LLUUID(gSavedSettings.getString("NearByChatChannelUUID")));
		if(chat_channel)
		{
			chat_channel->removeToastsFromChannel();
		}
	}

	LLDockableFloater::setVisible(visible);
}

void	LLNearbyChat::onOpen(const LLSD& key )
{
	LLDockableFloater::onOpen(key);
}

void LLNearbyChat::setRect	(const LLRect &rect)
{
	LLDockableFloater::setRect(rect);
}

void LLNearbyChat::getAllowedRect(LLRect& rect)
{
	rect = gViewerWindow->getWorldViewRectScaled();
}

void LLNearbyChat::updateChatHistoryStyle()
{
	mChatHistory->clear();
	for(std::vector<LLChat>::iterator it = mMessageArchive.begin();it!=mMessageArchive.end();++it)
	{
		addMessage(*it,false);
	}
}

//static 
void LLNearbyChat::processChatHistoryStyleUpdate(const LLSD& newvalue)
{
	LLNearbyChat* nearby_chat = LLFloaterReg::getTypedInstance<LLNearbyChat>("nearby_chat", LLSD());
	if(nearby_chat)
		nearby_chat->updateChatHistoryStyle();
}

bool isTwoWordsName(const std::string& name)
{
	//checking for a single space
	S32 pos = name.find(' ', 0);
	return std::string::npos != pos && name.rfind(' ', name.length()) == pos && 0 != pos && name.length()-1 != pos;
}

void LLNearbyChat::loadHistory()
{
	LLSD do_not_log;
	do_not_log["do_not_log"] = true;

	std::list<LLSD> history;
	LLLogChat::loadAllHistory("chat", history);

	std::list<LLSD>::const_iterator it = history.begin();
	while (it != history.end())
	{
		const LLSD& msg = *it;

		std::string from = msg[IM_FROM];
		LLUUID from_id = LLUUID::null;
		if (msg[IM_FROM_ID].isUndefined())
		{
			gCacheName->getUUID(from, from_id);
		}

		LLChat chat;
		chat.mFromName = from;
		chat.mFromID = from_id;
		chat.mText = msg[IM_TEXT].asString();
		chat.mTimeStr = msg[IM_TIME].asString();
		chat.mChatStyle = CHAT_STYLE_HISTORY;

		chat.mSourceType = CHAT_SOURCE_AGENT;
		if (from_id.isNull() && SYSTEM_FROM == from)
		{	
			chat.mSourceType = CHAT_SOURCE_SYSTEM;
			
		}
		else if (from_id.isNull())
		{
			chat.mSourceType = isTwoWordsName(from) ? CHAT_SOURCE_UNKNOWN : CHAT_SOURCE_OBJECT;
		}

		addMessage(chat, true, do_not_log);

		it++;
	}
}

//static
LLNearbyChat* LLNearbyChat::getInstance()
{
	return LLFloaterReg::getTypedInstance<LLNearbyChat>("nearby_chat", LLSD());
}

////////////////////////////////////////////////////////////////////////////////
//
void LLNearbyChat::onFocusReceived()
{
	setBackgroundOpaque(true);
	LLPanel::onFocusReceived();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLNearbyChat::onFocusLost()
{
	setBackgroundOpaque(false);
	LLPanel::onFocusLost();
}

