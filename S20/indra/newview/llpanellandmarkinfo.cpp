/**
 * @file llpanellandmarkinfo.cpp
 * @brief Displays landmark info in Side Tray.
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

#include "llpanellandmarkinfo.h"

#include "llcombobox.h"
#include "lliconctrl.h"
#include "lllineeditor.h"
#include "lltextbox.h"
#include "lltexteditor.h"
#include "lltrans.h"

#include "llagent.h"
#include "llagentui.h"
#include "lllandmarkactions.h"
#include "llviewerinventory.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"

//----------------------------------------------------------------------------
// Aux types and methods
//----------------------------------------------------------------------------

typedef std::pair<LLUUID, std::string> folder_pair_t;

static bool cmp_folders(const folder_pair_t& left, const folder_pair_t& right);
static void collectLandmarkFolders(LLInventoryModel::cat_array_t& cats);

static LLRegisterPanelClassWrapper<LLPanelLandmarkInfo> t_landmark_info("panel_landmark_info");

// Statics for textures filenames
static std::string icon_pg;
static std::string icon_m;
static std::string icon_r;

LLPanelLandmarkInfo::LLPanelLandmarkInfo()
:	LLPanelPlaceInfo()
{}

// virtual
LLPanelLandmarkInfo::~LLPanelLandmarkInfo()
{}

// virtual
BOOL LLPanelLandmarkInfo::postBuild()
{
	LLPanelPlaceInfo::postBuild();

	mOwner = getChild<LLTextBox>("owner");
	mCreator = getChild<LLTextBox>("creator");
	mCreated = getChild<LLTextBox>("created");

	mLandmarkTitle = getChild<LLTextBox>("title_value");
	mLandmarkTitleEditor = getChild<LLLineEditor>("title_editor");
	mNotesEditor = getChild<LLTextEditor>("notes_editor");
	mFolderCombo = getChild<LLComboBox>("folder_combo");

	icon_pg = getString("icon_PG");
	icon_m = getString("icon_M");
	icon_r = getString("icon_R");

	return TRUE;
}

// virtual
void LLPanelLandmarkInfo::resetLocation()
{
	LLPanelPlaceInfo::resetLocation();

	std::string loading = LLTrans::getString("LoadingData");
	mCreator->setText(loading);
	mOwner->setText(loading);
	mCreated->setText(loading);
	mLandmarkTitle->setText(LLStringUtil::null);
	mLandmarkTitleEditor->setText(LLStringUtil::null);
	mNotesEditor->setText(LLStringUtil::null);
}

// virtual
void LLPanelLandmarkInfo::setInfoType(EInfoType type)
{
	LLPanel* landmark_info_panel = getChild<LLPanel>("landmark_info_panel");

	bool is_info_type_create_landmark = type == CREATE_LANDMARK;

	landmark_info_panel->setVisible(type == LANDMARK);

	getChild<LLTextBox>("folder_label")->setVisible(is_info_type_create_landmark);
	mFolderCombo->setVisible(is_info_type_create_landmark);

	switch(type)
	{
		case CREATE_LANDMARK:
			mCurrentTitle = getString("title_create_landmark");

			mLandmarkTitle->setVisible(FALSE);
			mLandmarkTitleEditor->setVisible(TRUE);
			mNotesEditor->setEnabled(TRUE);
		break;

		case LANDMARK:
		default:
			mCurrentTitle = getString("title_landmark");

			mLandmarkTitle->setVisible(TRUE);
			mLandmarkTitleEditor->setVisible(FALSE);
			mNotesEditor->setEnabled(FALSE);
		break;
	}

	populateFoldersList();

	LLPanelPlaceInfo::setInfoType(type);
}

// virtual
void LLPanelLandmarkInfo::processParcelInfo(const LLParcelData& parcel_data)
{
	LLPanelPlaceInfo::processParcelInfo(parcel_data);

	// HACK: Flag 0x2 == adult region,
	// Flag 0x1 == mature region, otherwise assume PG
	if (parcel_data.flags & 0x2)
	{
		mMaturityRatingIcon->setValue(icon_r);
		mMaturityRatingText->setText(LLViewerRegion::accessToString(SIM_ACCESS_ADULT));
	}
	else if (parcel_data.flags & 0x1)
	{
		mMaturityRatingIcon->setValue(icon_m);
		mMaturityRatingText->setText(LLViewerRegion::accessToString(SIM_ACCESS_MATURE));
	}
	else
	{
		mMaturityRatingIcon->setValue(icon_pg);
		mMaturityRatingText->setText(LLViewerRegion::accessToString(SIM_ACCESS_PG));
	}

	S32 region_x;
	S32 region_y;
	S32 region_z;

	// If the region position is zero, grab position from the global
	if(mPosRegion.isExactlyZero())
	{
		region_x = llround(parcel_data.global_x) % REGION_WIDTH_UNITS;
		region_y = llround(parcel_data.global_y) % REGION_WIDTH_UNITS;
		region_z = llround(parcel_data.global_z);
	}
	else
	{
		region_x = llround(mPosRegion.mV[VX]);
		region_y = llround(mPosRegion.mV[VY]);
		region_z = llround(mPosRegion.mV[VZ]);
	}

	LLSD info;
	info["update_verbs"] = true;
	info["global_x"] = parcel_data.global_x;
	info["global_y"] = parcel_data.global_y;
	info["global_z"] = parcel_data.global_z;
	notifyParent(info);

	if (mInfoType == CREATE_LANDMARK)
	{
		if (parcel_data.name.empty())
		{
			mLandmarkTitleEditor->setText(llformat("%s (%d, %d, %d)",
								  parcel_data.sim_name.c_str(), region_x, region_y, region_z));
		}
		else
		{
			mLandmarkTitleEditor->setText(parcel_data.name);
		}

		std::string desc;
		LLAgentUI::buildLocationString(desc, LLAgentUI::LOCATION_FORMAT_FULL, gAgent.getPositionAgent());
		mNotesEditor->setText(desc);

		if (!LLLandmarkActions::landmarkAlreadyExists())
		{
			createLandmark(mFolderCombo->getValue().asUUID());
		}
	}
}

void LLPanelLandmarkInfo::displayItemInfo(const LLInventoryItem* pItem)
{
	if (!pItem)
		return;

	if(!gCacheName)
		return;

	const LLPermissions& perm = pItem->getPermissions();

	//////////////////
	// CREATOR NAME //
	//////////////////
	if (pItem->getCreatorUUID().notNull())
	{
		std::string name;
		LLUUID creator_id = pItem->getCreatorUUID();
		if (!gCacheName->getFullName(creator_id, name))
		{
			gCacheName->get(creator_id, FALSE,
							boost::bind(&LLPanelPlaceInfo::nameUpdatedCallback, mCreator, _2, _3));
		}
		mCreator->setText(name);
	}
	else
	{
		mCreator->setText(getString("unknown"));
	}

	////////////////
	// OWNER NAME //
	////////////////
	if(perm.isOwned())
	{
		std::string name;
		if (perm.isGroupOwned())
		{
			LLUUID group_id = perm.getGroup();
			if (!gCacheName->getGroupName(group_id, name))
			{
				gCacheName->get(group_id, TRUE,
								boost::bind(&LLPanelPlaceInfo::nameUpdatedCallback, mOwner, _2, _3));
			}
		}
		else
		{
			LLUUID owner_id = perm.getOwner();
			if (!gCacheName->getFullName(owner_id, name))
			{
				gCacheName->get(owner_id, FALSE,
								boost::bind(&LLPanelPlaceInfo::nameUpdatedCallback, mOwner, _2, _3));
			}
		}
		mOwner->setText(name);
	}
	else
	{
		mOwner->setText(getString("public"));
	}

	//////////////////
	// ACQUIRE DATE //
	//////////////////
	time_t time_utc = pItem->getCreationDate();
	if (0 == time_utc)
	{
		mCreated->setText(getString("unknown"));
	}
	else
	{
		std::string timeStr = getString("acquired_date");
		LLSD substitution;
		substitution["datetime"] = (S32) time_utc;
		LLStringUtil::format (timeStr, substitution);
		mCreated->setText(timeStr);
	}

	mLandmarkTitle->setText(pItem->getName());
	mLandmarkTitleEditor->setText(pItem->getName());
	mNotesEditor->setText(pItem->getDescription());
}

void LLPanelLandmarkInfo::toggleLandmarkEditMode(BOOL enabled)
{
	// If switching to edit mode while creating landmark
	// the "Create Landmark" title remains.
	if (enabled && mInfoType != CREATE_LANDMARK)
	{
		mTitle->setText(getString("title_edit_landmark"));
	}
	else
	{
		mTitle->setText(mCurrentTitle);

		mLandmarkTitle->setText(mLandmarkTitleEditor->getText());
	}

	if (mNotesEditor->getReadOnly() ==  (enabled == TRUE))
	{
		mLandmarkTitle->setVisible(!enabled);
		mLandmarkTitleEditor->setVisible(enabled);
		mNotesEditor->setReadOnly(!enabled);
		mFolderCombo->setVisible(enabled);
		getChild<LLTextBox>("folder_label")->setVisible(enabled);

		// HACK: To change the text color in a text editor
		// when it was enabled/disabled we set the text once again.
		mNotesEditor->setText(mNotesEditor->getText());
	}
}

const std::string& LLPanelLandmarkInfo::getLandmarkTitle() const
{
	return mLandmarkTitleEditor->getText();
}

const std::string LLPanelLandmarkInfo::getLandmarkNotes() const
{
	return mNotesEditor->getText();
}

const LLUUID LLPanelLandmarkInfo::getLandmarkFolder() const
{
	return mFolderCombo->getValue().asUUID();
}

BOOL LLPanelLandmarkInfo::setLandmarkFolder(const LLUUID& id)
{
	return mFolderCombo->setCurrentByID(id);
}

void LLPanelLandmarkInfo::createLandmark(const LLUUID& folder_id)
{
	std::string name = mLandmarkTitleEditor->getText();
	std::string desc = mNotesEditor->getText();

	LLStringUtil::trim(name);
	LLStringUtil::trim(desc);

	// If typed name is empty use the parcel name instead.
	if (name.empty())
	{
		name = mParcelName->getText();

		// If no parcel exists use the region name instead.
		if (name.empty())
		{
			name = mRegionName->getText();
		}
	}

	LLStringUtil::replaceChar(desc, '\n', ' ');

	// If no folder chosen use the "Landmarks" folder.
	LLLandmarkActions::createLandmarkHere(name, desc,
		folder_id.notNull() ? folder_id : gInventory.findCategoryUUIDForType(LLFolderType::FT_LANDMARK));
}

// static
std::string LLPanelLandmarkInfo::getFullFolderName(const LLViewerInventoryCategory* cat)
{
	std::string name;
	LLUUID parent_id;

	llassert(cat);
	if (cat)
	{
		name = cat->getName();

		// translate category name, if it's right below the root
		// FIXME: it can throw notification about non existent string in strings.xml
		if (cat->getParentUUID().notNull() && cat->getParentUUID() == gInventory.getRootFolderID())
		{
			LLTrans::findString(name, "InvFolder " + name);
		}

		// we don't want "My Inventory" to appear in the name
		while ((parent_id = cat->getParentUUID()).notNull() && parent_id != gInventory.getRootFolderID())
		{
			cat = gInventory.getCategory(parent_id);
			llassert(cat);
			if (cat)
			{
				name = cat->getName() + "/" + name;
			}
		}
	}

	return name;
}

void LLPanelLandmarkInfo::populateFoldersList()
{
	// Collect all folders that can contain landmarks.
	LLInventoryModel::cat_array_t cats;
	collectLandmarkFolders(cats);

	mFolderCombo->removeall();

	// Put the "Landmarks" folder first in list.
	LLUUID landmarks_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_LANDMARK);
	const LLViewerInventoryCategory* lmcat = gInventory.getCategory(landmarks_id);
	if (!lmcat)
	{
		llwarns << "Cannot find the landmarks folder" << llendl;
	}
	else
	{
		std::string cat_full_name = getFullFolderName(lmcat);
		mFolderCombo->add(cat_full_name, lmcat->getUUID());
	}

	typedef std::vector<folder_pair_t> folder_vec_t;
	folder_vec_t folders;
	// Sort the folders by their full name.
	for (S32 i = 0; i < cats.count(); i++)
	{
		const LLViewerInventoryCategory* cat = cats.get(i);
		std::string cat_full_name = getFullFolderName(cat);
		folders.push_back(folder_pair_t(cat->getUUID(), cat_full_name));
	}
	sort(folders.begin(), folders.end(), cmp_folders);

	// Finally, populate the combobox.
	for (folder_vec_t::const_iterator it = folders.begin(); it != folders.end(); it++)
		mFolderCombo->add(it->second, LLSD(it->first));
}

static bool cmp_folders(const folder_pair_t& left, const folder_pair_t& right)
{
	return left.second < right.second;
}

static void collectLandmarkFolders(LLInventoryModel::cat_array_t& cats)
{
	LLUUID landmarks_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_LANDMARK);

	// Add descendent folders of the "Landmarks" category.
	LLInventoryModel::item_array_t items; // unused
	LLIsType is_category(LLAssetType::AT_CATEGORY);
	gInventory.collectDescendentsIf(
		landmarks_id,
		cats,
		items,
		LLInventoryModel::EXCLUDE_TRASH,
		is_category);

	// Add the "My Favorites" category.
	LLUUID favorites_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_FAVORITE);
	LLViewerInventoryCategory* favorites_cat = gInventory.getCategory(favorites_id);
	if (!favorites_cat)
	{
		llwarns << "Cannot find the favorites folder" << llendl;
	}
	else
	{
		cats.put(favorites_cat);
	}
}
