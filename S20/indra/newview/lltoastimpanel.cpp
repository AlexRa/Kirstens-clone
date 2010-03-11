/**
 * @file lltoastimpanel.cpp
 * @brief Panel for IM toasts.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2010, Linden Research, Inc.
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
#include "lltoastimpanel.h"

#include "llagent.h"
#include "llfloaterreg.h"
#include "llgroupactions.h"
#include "llgroupiconctrl.h"
#include "llimview.h"
#include "llnotifications.h"
#include "llinstantmessage.h"
#include "lltooltip.h"

#include "llviewerchat.h"

const S32 LLToastIMPanel::DEFAULT_MESSAGE_MAX_LINE_COUNT	= 6;

//--------------------------------------------------------------------------
LLToastIMPanel::LLToastIMPanel(LLToastIMPanel::Params &p) :	LLToastPanel(p.notification),
								mAvatarIcon(NULL), mAvatarName(NULL),
								mTime(NULL), mMessage(NULL), mGroupIcon(NULL)
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_instant_message.xml");

	mGroupIcon = getChild<LLGroupIconCtrl>("group_icon");
	mAvatarIcon = getChild<LLAvatarIconCtrl>("avatar_icon");
	mAdhocIcon = getChild<LLAvatarIconCtrl>("adhoc_icon");
	mAvatarName = getChild<LLTextBox>("user_name");
	mTime = getChild<LLTextBox>("time_box");
	mMessage = getChild<LLTextBox>("message");

	LLStyle::Params style_params;
	LLFontGL* fontp = LLViewerChat::getChatFont();
	std::string font_name = LLFontGL::nameFromFont(fontp);
	std::string font_size = LLFontGL::sizeFromFont(fontp);
	style_params.font.name(font_name);
	style_params.font.size(font_size);
	
	
	//Handle IRC styled /me messages.
	std::string prefix = p.message.substr(0, 4);
	if (prefix == "/me " || prefix == "/me'")
	{
		//style_params.font.style = "UNDERLINE";
		mMessage->clear();
		
		style_params.font.style ="ITALIC";
		mMessage->appendText(p.from, FALSE, style_params);

		style_params.font.style = "ITALIC";
		mMessage->appendText(p.message.substr(3), FALSE, style_params);
	}
	else
	{
		style_params.font.style =  "NORMAL";
		mMessage->setText(p.message, style_params);
	}

	mAvatarName->setValue(p.from);
	mTime->setValue(p.time);
	mSessionID = p.session_id;
	mAvatarID = p.avatar_id;
	mNotification = p.notification;

	initIcon();

	S32 maxLinesCount;
	std::istringstream ss( getString("message_max_lines_count") );
	if (!(ss >> maxLinesCount))
	{
		maxLinesCount = DEFAULT_MESSAGE_MAX_LINE_COUNT;
	}
	snapToMessageHeight(mMessage, maxLinesCount);
}

//--------------------------------------------------------------------------
LLToastIMPanel::~LLToastIMPanel()
{
}

//virtual
BOOL LLToastIMPanel::handleMouseDown(S32 x, S32 y, MASK mask)
{
	if (LLPanel::handleMouseDown(x,y,mask) == FALSE)
	{
		mNotification->respond(mNotification->getResponseTemplate());
	}

	return TRUE;
}

//virtual
BOOL LLToastIMPanel::handleToolTip(S32 x, S32 y, MASK mask)
{
	// It's not our direct child, so parentPointInView() doesn't work.
	LLRect ctrl_rect;

	mAvatarName->localRectToOtherView(mAvatarName->getLocalRect(), &ctrl_rect, this);
	if (ctrl_rect.pointInRect(x, y))
	{
		spawnNameToolTip();
		return TRUE;
	}

	mGroupIcon->localRectToOtherView(mGroupIcon->getLocalRect(), &ctrl_rect, this);
	if(mGroupIcon->getVisible() && ctrl_rect.pointInRect(x, y))
	{
		spawnGroupIconToolTip();
		return TRUE;
	}

	return LLToastPanel::handleToolTip(x, y, mask);
}

void LLToastIMPanel::spawnNameToolTip()
{
	// Spawn at right side of the name textbox.
	LLRect sticky_rect = mAvatarName->calcScreenRect();
	S32 icon_x = llmin(sticky_rect.mLeft + mAvatarName->getTextPixelWidth() + 3, sticky_rect.mRight - 16);
	LLCoordGL pos(icon_x, sticky_rect.mTop);

	LLToolTip::Params params;
	params.background_visible(false);
	params.click_callback(boost::bind(&LLFloaterReg::showInstance, "inspect_avatar", LLSD().with("avatar_id", mAvatarID), FALSE));
	params.delay_time(0.0f);		// spawn instantly on hover
	params.image(LLUI::getUIImage("Info_Small"));
	params.message("");
	params.padding(0);
	params.pos(pos);
	params.sticky_rect(sticky_rect);

	LLToolTipMgr::getInstance()->show(params);
}

void LLToastIMPanel::spawnGroupIconToolTip()
{
	// Spawn at right bottom side of group icon.
	LLRect sticky_rect = mGroupIcon->calcScreenRect();
	LLCoordGL pos(sticky_rect.mRight, sticky_rect.mBottom);

	LLGroupData g_data;
	if(!gAgent.getGroupData(mSessionID, g_data))
	{
		llwarns << "Error getting group data" << llendl;
	}

	LLInspector::Params params;
	params.fillFrom(LLUICtrlFactory::instance().getDefaultParams<LLInspector>());
	params.click_callback(boost::bind(&LLFloaterReg::showInstance, "inspect_group", LLSD().with("group_id", mSessionID), FALSE));
	params.delay_time(0.100f);
	params.image(LLUI::getUIImage("Info_Small"));
	params.message(g_data.mName);
	params.padding(3);
	params.pos(pos);
	params.max_width(300);

	LLToolTipMgr::getInstance()->show(params);
}

void LLToastIMPanel::initIcon()
{
	mAvatarIcon->setVisible(FALSE);
	mGroupIcon->setVisible(FALSE);
	mAdhocIcon->setVisible(FALSE);

	if(mAvatarName->getValue().asString() == SYSTEM_FROM)
	{
		// "sys_msg_icon" was disabled by Erica in the changeset: 5109 (85181bc92cbe)
		// and "dummy widget" warnings appeared in log.
		// It does not make sense to have such image with empty name. Removed for EXT-5057.
	}
	else
	{
		LLIMModel::LLIMSession* im_session = LLIMModel::getInstance()->findIMSession(mSessionID);
		if(!im_session)
		{
			llwarns << "Invalid IM session" << llendl;
			return;
		}

		switch(im_session->mSessionType)
		{
		case LLIMModel::LLIMSession::P2P_SESSION:
			mAvatarIcon->setVisible(TRUE);
			mAvatarIcon->setValue(mAvatarID);
			break;
		case LLIMModel::LLIMSession::GROUP_SESSION:
			mGroupIcon->setVisible(TRUE);
			mGroupIcon->setValue(mSessionID);
			break;
		case LLIMModel::LLIMSession::ADHOC_SESSION:
			mAdhocIcon->setVisible(TRUE);
			mAdhocIcon->setValue(im_session->mOtherParticipantID);
			mAdhocIcon->setToolTip(im_session->mName);
			break;
		default:
			llwarns << "Unknown IM session type" << llendl;
			break;
		}
	}
}

// EOF
