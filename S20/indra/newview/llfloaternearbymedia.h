/** 
 * @file llfloaternearbymedia.h
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

#ifndef LL_LLFLOATERNEARBYMEDIA_H
#define LL_LLFLOATERNEARBYMEDIA_H

#include "llfloater.h"
#include "llpanel.h"

class LLPanelNearbyMedia;
class LLButton;
class LLScrollListCtrl;
class LLSliderCtrl;
class LLCheckBoxCtrl;
class LLTextBox;
class LLViewerMediaImpl;

class LLFloaterNearbyMedia : public LLFloater
{
	friend class LLFloaterReg;
public:
	
	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();

	// this is part of the nearby media *dialog* so we can track whether
	// the user *implicitly* wants audio on or off via their *explicit*
	// interaction with our buttons.
	bool getParcelAudioAutoStart();

private:
	
	enum ColumnIndex {
		PROXIMITY_COLUMN = 0,
		STOP_COLUMN = 1,
		NAME_COLUMN = 2,
		DEBUG_COLUMN = 3
	};
		
	LLFloaterNearbyMedia(const LLSD& seed);
	virtual ~LLFloaterNearbyMedia();

	// Add/remove an LLViewerMediaImpl to/from the list
	void addMediaItem(const LLUUID &id);
	void updateMediaItem(LLScrollListItem* item, LLViewerMediaImpl* impl);
	void removeMediaItem(const LLUUID &id);
	
	// Refresh the list in the UI
	void refreshList();
	
	void refreshParcelMediaUI();
	void refreshParcelAudioUI();

	// UI Callbacks 
	void onClickEnableAll();
	void onClickEnableParcelMedia();
	void onClickMuteParcelMedia();
	void onParcelMediaVolumeSlider();
	void onClickParcelMediaPlay();
	void onClickParcelMediaStop();
	void onClickParcelMediaPause();
	void onClickParcelAudioPlay();
	void onClickParcelAudioStop();
	void onClickParcelAudioPause();
	void onCheckAutoPlay();
	void onCheckEnableMedia();
	
	void onCheckItem(LLUICtrl* ctrl, const LLUUID &row_id);
	
	static void onDoubleClickMedia(void* user_data);
	
private:
	
	void setAllMediaState(bool enabled, bool tentative);
	
	bool setDisabled(const LLUUID &id, bool disabled);
	
	static void getNameAndUrlHelper(LLViewerMediaImpl* impl, std::string& name, std::string & url, const std::string &defaultName);

	static std::string getParcelAudioURL();
	
	LLScrollListCtrl*	mMediaList;
	LLUICtrl*			mEnableAllCtrl;
	LLSliderCtrl*		mParcelMediaVolumeSlider;
	LLButton*			mParcelMediaMuteCtrl;
	LLUICtrl*			mEnableParcelMediaCtrl;
	LLTextBox*			mParcelMediaText;
	LLTextBox*			mItemCountText;
	LLUICtrl*			mParcelMediaCtrl;
	LLUICtrl*			mParcelMediaPlayCtrl;
	LLUICtrl*			mParcelMediaPauseCtrl;
	LLUICtrl*			mParcelAudioCtrl;
	LLUICtrl*			mParcelAudioPlayCtrl;
	LLUICtrl*			mParcelAudioPauseCtrl;
	LLCheckBoxCtrl*		mAutoPlayCtrl;
	LLCheckBoxCtrl*		mMediaEnabledCtrl;
	
	bool				mAllMediaDisabled;
	bool				mDebugInfoVisible;
	bool				mParcelAudioAutoStart;
	std::string			mEmptyNameString;
	std::string			mDefaultParcelMediaName;
	std::string			mDefaultParcelMediaToolTip;
};


#endif // LL_LLFLOATERNEARBYMEDIA_H
