/** 
 * @file llimhandler.cpp
 * @brief Notification Handler Class for IM notifications
 *
 * $LicenseInfo:firstyear=2000&license=viewergpl$
 * 
 * Copyright (c) 2000-2010, Linden Research, Inc.
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


#include "llviewerprecompiledheaders.h" // must be first include

#include "llnotificationhandler.h"

#include "llagentdata.h"
#include "llnotifications.h"
#include "lltoastimpanel.h"
#include "llviewerwindow.h"

using namespace LLNotificationsUI;

//--------------------------------------------------------------------------
LLIMHandler::LLIMHandler(e_notification_type type, const LLSD& id)
{
	mType = type;

	// Getting a Channel for our notifications
	mChannel = LLChannelManager::getInstance()->createNotificationChannel();
}

//--------------------------------------------------------------------------
LLIMHandler::~LLIMHandler()
{
}

//--------------------------------------------------------------------------
void LLIMHandler::initChannel()
{
	S32 channel_right_bound = gViewerWindow->getWorldViewRectScaled().mRight - gSavedSettings.getS32("NotificationChannelRightMargin"); 
	S32 channel_width = gSavedSettings.getS32("NotifyBoxWidth");
	mChannel->init(channel_right_bound - channel_width, channel_right_bound);
}

//--------------------------------------------------------------------------
bool LLIMHandler::processNotification(const LLSD& notify)
{
	if(!mChannel)
	{
		return false;
	}

	LLNotificationPtr notification = LLNotifications::instance().find(notify["id"].asUUID());

	if(!notification)
		return false;

	// arrange a channel on a screen
	if(!mChannel->getVisible())
	{
		initChannel();
	}

	if(notify["sigtype"].asString() == "add" || notify["sigtype"].asString() == "change")
	{
		LLSD substitutions = notification->getSubstitutions();

		// According to comments in LLIMMgr::addMessage(), if we get message
		// from ourselves, the sender id is set to null. This fixes EXT-875.
		LLUUID avatar_id = substitutions["FROM_ID"].asUUID();
		if (avatar_id.isNull())
			avatar_id = gAgentID;

		LLToastIMPanel::Params im_p;
		im_p.notification = notification;
		im_p.avatar_id = avatar_id;
		im_p.from = substitutions["FROM"].asString();
		im_p.time = substitutions["TIME"].asString();
		im_p.message = substitutions["MESSAGE"].asString();
		im_p.session_id = substitutions["SESSION_ID"].asUUID();

		LLToastIMPanel* im_box = new LLToastIMPanel(im_p);

		LLToast::Params p;
		p.notif_id = notification->getID();
		p.session_id = im_p.session_id;
		p.notification = notification;
		p.panel = im_box;
		p.can_be_stored = false;
		p.on_delete_toast = boost::bind(&LLIMHandler::onDeleteToast, this, _1);
		LLScreenChannel* channel = dynamic_cast<LLScreenChannel*>(mChannel);
		if(channel)
			channel->addToast(p);

		// send a signal to the counter manager;
		mNewNotificationSignal();
	}
	else if (notify["sigtype"].asString() == "delete")
	{
		mChannel->killToastByNotificationID(notification->getID());
	}
	return true;
}

//--------------------------------------------------------------------------
void LLIMHandler::onDeleteToast(LLToast* toast)
{
	// send a signal to the counter manager
	mDelNotificationSignal();
}

//--------------------------------------------------------------------------


