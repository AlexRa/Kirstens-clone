/** 
 * @file llfloaternearbymedia.cpp
 * @brief Management interface for muting and controlling nearby media
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2010, Linden Research, Inc.
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

#include "llfloaternearbymedia.h"

#include "llaudioengine.h"
#include "llcheckboxctrl.h"
#include "llscrolllistctrl.h"
#include "llscrolllistitem.h"
#include "llscrolllistcell.h"
#include "llsliderctrl.h"
#include "llagent.h"
#include "llagentui.h"
#include "llbutton.h"
#include "lltextbox.h"
#include "llviewermedia.h"
#include "llviewerparcelmedia.h"
#include "llviewerregion.h"
#include "llviewermediafocus.h"
#include "llviewerparcelmgr.h"
#include "llparcel.h"
#include "llpluginclassmedia.h"

#include <boost/lexical_cast.hpp>

extern LLControlGroup gSavedSettings;

// Ugh, isInternetStreamPlaying() returns not a bool, but an *int*, with
// 0 = stopped, 1 = playing, 2 = paused.
static const int PARCEL_AUDIO_STOPPED = 0;
static const int PARCEL_AUDIO_PLAYING = 1;
static const int PARCEL_AUDIO_PAUSED = 2;

//
// LLFloaterNearbyMedia
//

LLFloaterNearbyMedia::LLFloaterNearbyMedia(const LLSD& seed)
	: LLFloater(seed),
	  mMediaList(NULL),
	  mEnableAllCtrl(NULL),
	  mEnableParcelMediaCtrl(NULL),	  
	  mAllMediaDisabled(false),
	  mDebugInfoVisible(false)
{
	mParcelAudioAutoStart = gSavedSettings.getBOOL(LLViewerMedia::AUTO_PLAY_MEDIA_SETTING);

	mCommitCallbackRegistrar.add("MediaListCtrl.EnableAll",		boost::bind(&LLFloaterNearbyMedia::onClickEnableAll, this));
	mCommitCallbackRegistrar.add("NearbyMedia.EnableMedia",		boost::bind(&LLFloaterNearbyMedia::onCheckEnableMedia, this));
	mCommitCallbackRegistrar.add("NearbyMedia.AutoPlay",		boost::bind(&LLFloaterNearbyMedia::onCheckAutoPlay, this));
	mCommitCallbackRegistrar.add("ParcelMediaCtrl.ParcelMediaVolume",		boost::bind(&LLFloaterNearbyMedia::onParcelMediaVolumeSlider, this));
	mCommitCallbackRegistrar.add("ParcelMediaCtrl.MuteParcelMedia",		boost::bind(&LLFloaterNearbyMedia::onClickMuteParcelMedia, this));
	mCommitCallbackRegistrar.add("ParcelMediaCtrl.EnableParcelMedia",		boost::bind(&LLFloaterNearbyMedia::onClickEnableParcelMedia, this));
	mCommitCallbackRegistrar.add("ParcelMediaCtrl.Play",		boost::bind(&LLFloaterNearbyMedia::onClickParcelMediaPlay, this));
	mCommitCallbackRegistrar.add("ParcelMediaCtrl.Stop",		boost::bind(&LLFloaterNearbyMedia::onClickParcelMediaStop, this));
	mCommitCallbackRegistrar.add("ParcelMediaCtrl.Pause",		boost::bind(&LLFloaterNearbyMedia::onClickParcelMediaPause, this));
	mCommitCallbackRegistrar.add("ParcelAudioCtrl.Play",		boost::bind(&LLFloaterNearbyMedia::onClickParcelAudioPlay, this));
	mCommitCallbackRegistrar.add("ParcelAudioCtrl.Stop",		boost::bind(&LLFloaterNearbyMedia::onClickParcelAudioStop, this));
	mCommitCallbackRegistrar.add("ParcelAudioCtrl.Pause",		boost::bind(&LLFloaterNearbyMedia::onClickParcelAudioPause, this));
}

LLFloaterNearbyMedia::~LLFloaterNearbyMedia()
{
}

BOOL LLFloaterNearbyMedia::postBuild()
{
	mMediaList = getChild<LLScrollListCtrl>("media_list");
	mEnableAllCtrl = getChild<LLUICtrl>("all_nearby_media_enable_btn");
	mParcelMediaVolumeSlider = getChild<LLSliderCtrl>("parcel_media_volume");
	mParcelMediaMuteCtrl = getChild<LLButton>("parcel_media_mute");
	mEnableParcelMediaCtrl = getChild<LLUICtrl>("parcel_media_enable_btn");
	mParcelMediaText = getChild<LLTextBox>("parcel_media_name");
	mItemCountText = getChild<LLTextBox>("media_item_count");
	mAutoPlayCtrl = getChild<LLCheckBoxCtrl>("media_auto_play_btn");
	mAutoPlayCtrl->set(gSavedSettings.getBOOL(LLViewerMedia::AUTO_PLAY_MEDIA_SETTING));
	mMediaEnabledCtrl = getChild<LLCheckBoxCtrl>("media_enabled_btn");
	mMediaEnabledCtrl->set(gSavedSettings.getBOOL("AudioStreamingMedia"));
	mParcelMediaPlayCtrl = getChild<LLButton>("parcel_media_play_btn");
	mParcelMediaPauseCtrl = getChild<LLButton>("parcel_media_pause_btn");
	mParcelMediaCtrl = getChild<LLUICtrl>("parcel_media_ctrls");
	mParcelAudioPlayCtrl = getChild<LLButton>("parcel_audio_play_btn");
	mParcelAudioPauseCtrl = getChild<LLButton>("parcel_audio_pause_btn");
	mParcelAudioCtrl = getChild<LLUICtrl>("parcel_audio_ctrls");
	
	mEmptyNameString = getString("empty_item_text");
	mDefaultParcelMediaName = getString("default_parcel_media_name");
	mDefaultParcelMediaToolTip = getString("default_parcel_media_tool_tip");
	
	setAllMediaState(! LLViewerMedia::getInWorldMediaDisabled(), false);
	
	mMediaList->setDoubleClickCallback(onDoubleClickMedia, this);
	mMediaList->sortByColumnIndex(PROXIMITY_COLUMN, TRUE);

	std::string url = LLViewerParcelMedia::getURL();
	if (!url.empty())
	{
		std::string name = LLViewerParcelMedia::getName();
		mParcelMediaText->setValue(name.empty()?url:name);
		mParcelMediaText->setToolTip(url);	
	}
	refreshList();
	
	return TRUE;
}

void LLFloaterNearbyMedia::draw()
{
	mItemCountText->setValue(llformat(getString("media_item_count_format").c_str(), mMediaList->getItemCount()));
	mAutoPlayCtrl->set(gSavedSettings.getBOOL(LLViewerMedia::AUTO_PLAY_MEDIA_SETTING));
	mMediaEnabledCtrl->set(gSavedSettings.getBOOL("AudioStreamingMedia"));
	
	refreshParcelMediaUI();
	refreshParcelAudioUI();
	refreshList();
	
	LLFloater::draw();
}

bool LLFloaterNearbyMedia::getParcelAudioAutoStart()
{
	return mParcelAudioAutoStart;
}

void LLFloaterNearbyMedia::addMediaItem(const LLUUID &id)
{
	if (NULL == mMediaList) return;
	
	// Just set up the columns -- the values will be filled in by updateMediaItem().
	
	LLSD row;
	row["id"] = id;
	
	LLSD &columns = row["columns"];
	columns[PROXIMITY_COLUMN]["column"] = "media_proximity";
	columns[PROXIMITY_COLUMN]["value"] = "";
	columns[STOP_COLUMN]["column"] = "media_stop_ctrl";
	columns[STOP_COLUMN]["type"] = "checkbox";
	columns[NAME_COLUMN]["column"] = "media_name";
	columns[NAME_COLUMN]["type"] = "text";
	columns[NAME_COLUMN]["value"] = "";
	if(mDebugInfoVisible)
	{
		columns[DEBUG_COLUMN]["column"] = "debug";
		columns[DEBUG_COLUMN]["type"] = "text";
		columns[DEBUG_COLUMN]["value"] = "";
	}
	
	LLScrollListItem* new_item = mMediaList->addElement(row);
	LLScrollListCheck* scroll_list_check = dynamic_cast<LLScrollListCheck*>(new_item->getColumn(STOP_COLUMN));
	if (scroll_list_check)
	{
		LLCheckBoxCtrl *check = scroll_list_check->getCheckBox();
		check->setCommitCallback(boost::bind(&LLFloaterNearbyMedia::onCheckItem, this, _1, id));
	}
}

void LLFloaterNearbyMedia::updateMediaItem(LLScrollListItem* item, LLViewerMediaImpl* impl)
{
	LLScrollListCell* cell = item->getColumn(PROXIMITY_COLUMN);
	if(cell)
	{
		// since we are forced to sort by text, encode sort order as string
		std::string proximity_string = llformat("%010d", impl->getProximity());
		std::string old_proximity_string = cell->getValue().asString();
		if(proximity_string != old_proximity_string)
		{
			cell->setValue(proximity_string);
			mMediaList->setNeedsSort(true);
		}
	}
	
	cell = item->getColumn(STOP_COLUMN);
	if(cell)
	{
		cell->setValue(!impl->isMediaDisabled());
	}
	
	cell = item->getColumn(NAME_COLUMN);
	if(cell)
	{
		std::string name;
		std::string url;
		
		getNameAndUrlHelper(impl, name, url, mEmptyNameString);
		
		cell->setToolTip(url);
		cell->setColor(impl->getPriority() == LLPluginClassMedia::PRIORITY_UNLOADED ?
					   LLColor4::grey : LLColor4::white );
		
		std::string old_name = cell->getValue().asString();
		if(old_name != name)
		{
			cell->setValue(name);
		}
	}
	
	if(mDebugInfoVisible)
	{
		cell = item->getColumn(DEBUG_COLUMN);
		if(cell)
		{
			std::string s;
			
			s += llformat("%g/", (float)impl->getInterest());

			// proximity distance is actually distance squared -- display it as straight distance.
			s += llformat("%g/", fsqrtf(impl->getProximityDistance()));

//			s += llformat("%g/", (float)impl->getCPUUsage());
//			s += llformat("%g/", (float)impl->getApproximateTextureInterest());
			
			s += LLPluginClassMedia::priorityToString(impl->getPriority());
			
			if(impl->hasMedia())
			{
				s += '@';
			}
			else if(impl->isPlayable())
			{
				s += '+';
			}
			else if(impl->isForcedUnloaded())
			{
				s += '!';
			}
				
			cell->setValue(s);
		}
	}
}

void LLFloaterNearbyMedia::removeMediaItem(const LLUUID &id)
{
	if (NULL == mMediaList) return;
	
	mMediaList->deleteSingleItem(mMediaList->getItemIndex(id));
}

void LLFloaterNearbyMedia::refreshParcelMediaUI()
{	
	std::string url = LLViewerParcelMedia::getURL();
	LLStyle::Params style_params;
	if (url.empty())
	{	
		style_params.font.style = "ITALIC";
		mParcelMediaText->setText(mDefaultParcelMediaName, style_params);
		mParcelMediaText->setToolTip(mDefaultParcelMediaToolTip);	
		mEnableParcelMediaCtrl->setEnabled(false);
		mEnableParcelMediaCtrl->setValue(false);
	}
	else {
		std::string name = LLViewerParcelMedia::getName();
		if (name.empty()) name = url;
		mParcelMediaText->setText(name, style_params);
		mParcelMediaText->setToolTip(url);
		mEnableParcelMediaCtrl->setEnabled(true);
		mEnableParcelMediaCtrl->setValue(false);
	}
	
	// Set up the default play controls state
	mParcelMediaPauseCtrl->setEnabled(false);
	mParcelMediaPauseCtrl->setVisible(false);
	mParcelMediaPlayCtrl->setEnabled(true);
	mParcelMediaPlayCtrl->setVisible(true);
	mParcelMediaCtrl->setEnabled(false);
	
	if (LLViewerParcelMedia::getParcelMedia())
	{
		mEnableParcelMediaCtrl->setValue(! LLViewerParcelMedia::getParcelMedia()->isMediaDisabled());
		
		if (LLViewerParcelMedia::getParcelMedia()->getMediaPlugin() &&
			LLViewerParcelMedia::getParcelMedia()->getMediaPlugin()->pluginSupportsMediaTime())
		{
			mParcelMediaCtrl->setEnabled(true);
			
			switch(LLViewerParcelMedia::getParcelMedia()->getMediaPlugin()->getStatus())
			{
				case LLPluginClassMediaOwner::MEDIA_PLAYING:
					mParcelMediaPlayCtrl->setEnabled(false);
					mParcelMediaPlayCtrl->setVisible(false);
					mParcelMediaPauseCtrl->setEnabled(true);
					mParcelMediaPauseCtrl->setVisible(true);
					break;
				case LLPluginClassMediaOwner::MEDIA_PAUSED:
				default:
					// default play status is kosher
					break;
			}
		}
	}
}

void LLFloaterNearbyMedia::refreshParcelAudioUI()
{	
	bool parcel_audio_enabled = !getParcelAudioURL().empty();

	mParcelAudioCtrl->setToolTip(getParcelAudioURL());
	
	if (gAudiop && parcel_audio_enabled)
	{
		mParcelAudioCtrl->setEnabled(true);

		if (PARCEL_AUDIO_PLAYING == gAudiop->isInternetStreamPlaying())
		{
			mParcelAudioPlayCtrl->setEnabled(false);
			mParcelAudioPlayCtrl->setVisible(false);
			mParcelAudioPauseCtrl->setEnabled(true);
			mParcelAudioPauseCtrl->setVisible(true);
		}
		else {
			mParcelAudioPlayCtrl->setEnabled(true);
			mParcelAudioPlayCtrl->setVisible(true);
			mParcelAudioPauseCtrl->setEnabled(false);
			mParcelAudioPauseCtrl->setVisible(false);
		}
	}
	else {
		mParcelAudioCtrl->setEnabled(false);
		mParcelAudioPlayCtrl->setEnabled(true);
		mParcelAudioPlayCtrl->setVisible(true);
		mParcelAudioPauseCtrl->setEnabled(false);
		mParcelAudioPauseCtrl->setVisible(false);
	}
}

void LLFloaterNearbyMedia::refreshList()
{
	bool all_items_deleted = false;
		
	if(!mMediaList)
	{
		// None of this makes any sense if the media list isn't there.
		return;
	}
	
	// Check whether the debug column has been shown/hidden.
	bool debug_info_visible = gSavedSettings.getBOOL("MediaPerformanceManagerDebug");
	if(debug_info_visible != mDebugInfoVisible)
	{
		mDebugInfoVisible = debug_info_visible;

		// Clear all items so the list gets regenerated.
		mMediaList->deleteAllItems();
		all_items_deleted = true;
	}
	
	// Get the canonical list from LLViewerMedia
	LLViewerMedia::impl_list impls = LLViewerMedia::getPriorityList();
	LLViewerMedia::impl_list::iterator priority_iter;
	
	// iterate over the impl list, creating rows as necessary.
	for(priority_iter = impls.begin(); priority_iter != impls.end(); priority_iter++)
	{
		// If we just emptied out the list, every flag needs to be reset.
		if(all_items_deleted)
		{
			(*priority_iter)->setInNearbyMediaList(false);
		}
		
		if(! (*priority_iter)->isParcelMedia())
		{
			LLUUID media_id = (*priority_iter)->getMediaTextureID();
			S32 proximity = (*priority_iter)->getProximity();
			// This is expensive (i.e. a linear search) -- don't use it here.  We now use mInNearbyMediaList instead.
//			S32 index = mMediaList->getItemIndex(media_id);

			if(proximity < 0)
			{
				// This isn't inworld media -- don't show it in the list.
				if ((*priority_iter)->getInNearbyMediaList())
				{
					// There's a row for this impl -- remove it.
					removeMediaItem(media_id);
					(*priority_iter)->setInNearbyMediaList(false);
				}
			}
			else
			{
				if (!(*priority_iter)->getInNearbyMediaList())
				{
					// We don't have a row for this impl -- add one.
					addMediaItem(media_id);
					(*priority_iter)->setInNearbyMediaList(true);
				}
			}
		}
	}
	
	int disabled_count = 0;
	int enabled_count = 0;

	// Iterate over the rows in the control, updating ones whose impl exists, and deleting ones whose impl has gone away.
	std::vector<LLScrollListItem*> items = mMediaList->getAllData();

	for (std::vector<LLScrollListItem*>::iterator item_it = items.begin();
		item_it != items.end();
		++item_it)
	{
		LLScrollListItem* item = (*item_it);
		LLUUID row_id = item->getUUID();
		
		LLViewerMediaImpl* impl = LLViewerMedia::getMediaImplFromTextureID(row_id);
		if(impl)
		{
			updateMediaItem(item, impl);
			if(impl->isMediaDisabled())
				disabled_count++;
			else
				enabled_count++;
		}
		else
		{
			// This item's impl has been deleted -- remove the row.
			// Removing the row won't throw off our iteration, since we have a local copy of the array.
			// We just need to make sure we don't access this item after the delete.
			removeMediaItem(row_id);
		}
	}
	
	// If all media is disabled, the button will enable all media.
	// Otherwise, it will disable all media.
	mAllMediaDisabled = ((disabled_count > 0) && (enabled_count == 0));
	bool some_disabled = ((disabled_count > 0) && (enabled_count > 0));
	setAllMediaState(! mAllMediaDisabled, some_disabled);
	
	// Set the selection to whatever media impl the media focus/hover is on. 
	// This is an experiment, and can be removed by ifdefing out these 4 lines.
	LLUUID media_target = LLViewerMediaFocus::getInstance()->getControlsMediaID();
	if(media_target.notNull())
	{
		mMediaList->selectByID(media_target);
	}
}

void LLFloaterNearbyMedia::setAllMediaState(bool enabled, bool tentative)
{
	if (dynamic_cast<LLButton*>(mEnableAllCtrl))
	{
		dynamic_cast<LLButton*>(mEnableAllCtrl)->setToggleState(enabled);
	}
	else if (dynamic_cast<LLCheckBoxCtrl*> (mEnableAllCtrl))
	{
		dynamic_cast<LLCheckBoxCtrl*>(mEnableAllCtrl)->setValue(enabled);
	}
	mEnableAllCtrl->setTentative(tentative);
}

void LLFloaterNearbyMedia::onClickEnableAll()
{	
	bool disabled = !mAllMediaDisabled;
	
	// Iterate over the rows in the control, setting the disable flag on the impl for each.
	std::vector<LLScrollListItem*> items = mMediaList->getAllData();
	
	for (std::vector<LLScrollListItem*>::iterator item_it = items.begin();
		 item_it != items.end();
		 ++item_it)
	{
		setDisabled((*item_it)->getUUID(), disabled);
	}
	
	// Now do the parcel media	
	if (LLViewerParcelMedia::getParcelMedia())
	{
		LLViewerParcelMedia::getParcelMedia()->setDisabled(disabled);
	}
	
	// The hilight state of the control will be adjusted the next time through refreshList().
}

void LLFloaterNearbyMedia::onClickEnableParcelMedia()
{	
	if (mEnableParcelMediaCtrl->getValue())
	{
		LLViewerParcelMedia::play(LLViewerParcelMgr::getInstance()->getAgentParcel());
	}
	else {
		// This actually unloads the impl, as opposed to "stop"ping the media
		LLViewerParcelMedia::stop();
	}
}

void LLFloaterNearbyMedia::onCheckItem(LLUICtrl* ctrl, const LLUUID &row_id)
{	
	LLCheckBoxCtrl* check = static_cast<LLCheckBoxCtrl*>(ctrl);

	setDisabled(row_id, ! check->getValue());
}

bool LLFloaterNearbyMedia::setDisabled(const LLUUID &row_id, bool disabled)
{
	LLViewerMediaImpl* impl = LLViewerMedia::getMediaImplFromTextureID(row_id);
	if(impl)
	{
		impl->setDisabled(disabled);
		return true;
	}
	return false;
}
								
//static
void LLFloaterNearbyMedia::onDoubleClickMedia(void* user_data)
{
	LLFloaterNearbyMedia* panelp = (LLFloaterNearbyMedia*)user_data;
	LLUUID media_id = panelp->mMediaList->getValue().asUUID();
	
	LLViewerMediaFocus::getInstance()->focusZoomOnMedia(media_id);
}

void LLFloaterNearbyMedia::onClickMuteParcelMedia()
{
	if (LLViewerParcelMedia::getParcelMedia())
	{
		bool muted = mParcelMediaMuteCtrl->getValue();
		LLViewerParcelMedia::getParcelMedia()->setVolume(muted ? (F32)0 : mParcelMediaVolumeSlider->getValueF32() );
	}
}

void LLFloaterNearbyMedia::onParcelMediaVolumeSlider()
{
	if (LLViewerParcelMedia::getParcelMedia())
	{
		LLViewerParcelMedia::getParcelMedia()->setVolume(mParcelMediaVolumeSlider->getValueF32());
	}
}

void LLFloaterNearbyMedia::onClickParcelMediaPlay()
{
	LLViewerParcelMedia::play(LLViewerParcelMgr::getInstance()->getAgentParcel());
}

void LLFloaterNearbyMedia::onClickParcelMediaStop()
{	
	if (LLViewerParcelMedia::getParcelMedia())
	{
		// This stops the media playing, as opposed to unloading it like
		// LLViewerParcelMedia::stop() does
		LLViewerParcelMedia::getParcelMedia()->stop();
	}
}

void LLFloaterNearbyMedia::onClickParcelMediaPause()
{
	LLViewerParcelMedia::pause();
}

void LLFloaterNearbyMedia::onClickParcelAudioPlay()
{
	// User *explicitly* started the internet stream, so keep the stream
	// playing and updated as they cross to other parcels etc.
	mParcelAudioAutoStart = true;

	if (!gAudiop)
		return;

	if (PARCEL_AUDIO_PAUSED == gAudiop->isInternetStreamPlaying())
	{
		// 'false' means unpause
		gAudiop->pauseInternetStream(false);
	}
	else {
		gAudiop->startInternetStream(getParcelAudioURL());
	}
}

void LLFloaterNearbyMedia::onClickParcelAudioStop()
{
	// User *explicitly* stopped the internet stream, so don't
	// re-start audio when i.e. they move to another parcel, until
	// they explicitly start it again.
	mParcelAudioAutoStart = false;

	if (!gAudiop)
		return;

	gAudiop->stopInternetStream();
}

void LLFloaterNearbyMedia::onClickParcelAudioPause()
{
	if (!gAudiop)
		return;

	// 'true' means pause
	gAudiop->pauseInternetStream(true);
}

void LLFloaterNearbyMedia::onCheckAutoPlay()
{	
	gSavedSettings.setBOOL(LLViewerMedia::AUTO_PLAY_MEDIA_SETTING, mAutoPlayCtrl->getValue());
}

void LLFloaterNearbyMedia::onCheckEnableMedia()
{	
	gSavedSettings.setBOOL("AudioStreamingMedia", mMediaEnabledCtrl->getValue());
}


// static
void LLFloaterNearbyMedia::getNameAndUrlHelper(LLViewerMediaImpl* impl, std::string& name, std::string & url, const std::string &defaultName)
{
	if (NULL == impl) return;
	
	name = impl->getName();
	url = impl->getCurrentMediaURL();	// This is the URL the media impl actually has loaded
	if (url.empty())
	{
		url = impl->getMediaEntryURL();	// This is the current URL from the media data
	}
	if (url.empty())
	{
		url = impl->getHomeURL();		// This is the home URL from the media data
	}
	if (name.empty())
	{
		name = url;
	}
	if (name.empty())
	{
		name = defaultName;
	}
}

// static
std::string LLFloaterNearbyMedia::getParcelAudioURL()
{
	return LLViewerParcelMgr::getInstance()->getAgentParcel()->getMusicURL();
}


