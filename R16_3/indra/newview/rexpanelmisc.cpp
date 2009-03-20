/**
 * @file rexpanelmisc.cpp
 * @brief RexPanelMisc class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

// file include
#include "rexpanelmisc.h"

#include "llselectmgr.h"
#include "llvovolume.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llfloatermeshedit.h"
#include "llfocusmgr.h"
#include "llcombobox.h"
#include "llinventory.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"

RexPanelMisc::RexPanelMisc(const std::string& name)
	:	RexPanelPrim(name)
{
	setMouseOpaque(FALSE);
}

// virtual
RexPanelMisc::~RexPanelMisc()
{
	// Children all cleaned up by default view destructor.
}

// virtual
BOOL RexPanelMisc::postBuild()
{	
	refresh();

    childSetCommitCallback("RexClassName Ctrl", onCommitClassName, this);
    childSetCommitCallback("RexSoundName Ctrl", onCommitSoundID, this);
    childSetCommitCallback("RexSoundVolume Ctrl", onCommitSoundVolume, this);
    childSetCommitCallback("RexSoundRadius Ctrl", onCommitSoundRadius, this);
    childSetCommitCallback("RexSelectionPriority Ctrl", onCommitSelectionPriority, this);

	return TRUE;
}

void RexPanelMisc::enableCtrls(bool enabled)
{
    childSetEnabled("RexClassName", enabled);
    childSetEnabled("RexClassName Ctrl", enabled);
    childSetEnabled("RexSoundName", enabled);
    childSetEnabled("RexSoundName Ctrl", enabled);
    childSetEnabled("RexSoundVolume Ctrl", enabled);
    childSetEnabled("RexSoundRadius Ctrl", enabled);
    childSetEnabled("RexSelectionPriority Ctrl", enabled);
}

// virtual
void RexPanelMisc::getState()
{
    if (!mObject) 
        return;

    LLVOVolume *volobjp = (LLVOVolume *)mObject;

    getAssetNames();

    // Get the actual values
    childSetValue("RexClassName Ctrl", volobjp->getRexClassName());
    childSetValue("RexSoundName Ctrl", getSoundIndex(volobjp));
    childSetValue("RexSoundVolume Ctrl", volobjp->getRexSoundVolume());
    childSetValue("RexSoundRadius Ctrl", volobjp->getRexSoundRadius());
    childSetValue("RexSelectionPriority Ctrl", volobjp->getRexSelectionPriority());
}

void RexPanelMisc::onCommitClassName( LLUICtrl* ctrl, void* userdata )
{
	RexPanelMisc* self = (RexPanelMisc*) userdata;

    if (self->mObject)
    {
        LLString value = self->childGetValue("RexClassName Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexClassName(value);
        self->getState();
    }
}


void RexPanelMisc::onCommitSoundVolume( LLUICtrl* ctrl, void* userdata )
{
	RexPanelMisc* self = (RexPanelMisc*) userdata;

    if (self->mObject)
    {
        F32 value = self->childGetValue("RexSoundVolume Ctrl").asReal();
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexSoundVolume(value);
        self->getState();
    }
}

void RexPanelMisc::onCommitSoundRadius( LLUICtrl* ctrl, void* userdata )
{
	RexPanelMisc* self = (RexPanelMisc*) userdata;

    if (self->mObject)
    {
        F32 value = self->childGetValue("RexSoundRadius Ctrl").asReal();
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexSoundRadius(value);
        self->getState();
    }
}

void RexPanelMisc::onCommitSoundID( LLUICtrl* ctrl, void* userdata )
{
	RexPanelMisc* self = (RexPanelMisc*) userdata;

    if (self->mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;

        S32 index = self->childGetValue("RexSoundName Ctrl");
        if (index == -1) return; // Do not send "unknown"

        LLUUID id = LLUUID::null;
        if ((index >= 0) && (index < (S32)self->mSoundNames.size()))
        {
            id = self->mSoundNames[index].mUUID;
        }
        volobjp->setRexSoundID(id);

        self->getState();
    }
}

void RexPanelMisc::onCommitSelectionPriority( LLUICtrl* ctrl, void* userdata )
{
	RexPanelMisc* self = (RexPanelMisc*) userdata;

    if (self->mObject)
    {
        int value = self->childGetValue("RexSelectionPriority Ctrl").asInteger();
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexSelectionPriority(value);
        self->getState();
    }
}

S32 RexPanelMisc::getSoundIndex(LLVOVolume *volobjp)
{
	const LLUUID& soundId = volobjp->getRexSoundID();

	for (U32 i = 0; i < mSoundNames.size(); ++i)
	{
		if (mSoundNames[i].mUUID == soundId)
		{
			return i;
		}
	}

	return -1;
}

void RexPanelMisc::getAssetNames()
{
	LLComboBox* comboBox = getChild<LLComboBox>("RexSoundName Ctrl");
	if (!comboBox) return;

	comboBox->removeall();
	comboBox->add(LLString("<Unknown>"), -1);
	mSoundNames.clear();

	// Add null sound selection first
	{
		RexPanelSelection temp;
		temp.mUUID = LLUUID::null;
		temp.mName = "<None>";
		comboBox->add(temp.mName, (S32)mSoundNames.size());
		mSoundNames.push_back(temp);
	}

	// Then add all sound assets to combobox
	{
		LLViewerInventoryCategory::cat_array_t cats;
		LLViewerInventoryItem::item_array_t items;
		LLIsType isType(LLAssetType::AT_SOUND);
		gInventory.collectDescendentsIf(LLUUID::null,
								cats,
								items,
								LLInventoryModel::INCLUDE_TRASH,
								isType);

		for (S32 i = 0; i < items.count(); i++)
		{
			LLInventoryItem* itemp = items[i];
			RexPanelSelection temp;
			temp.mUUID = itemp->getAssetUUID();
			temp.mName = itemp->getName();
			comboBox->add(temp.mName, (S32)mSoundNames.size());
			mSoundNames.push_back(temp);
		}
	}

    comboBox->sortByName();
}

BOOL RexPanelMisc::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 LLString& tooltip_msg)
{
	BOOL handled = FALSE;

    LLComboBox*	sound = getChild<LLComboBox>("RexSoundName Ctrl");
    if (!sound) return handled;

    LLViewerObject* objectp = mObject;
    if (!objectp) return handled;
    LLVOVolume *volobjp = (LLVOVolume *)objectp;

    if (sound->getRect().pointInRect(x,y))
    {
        if (cargo_type == DAD_SOUND)
        {
            LLInventoryItem* item = (LLInventoryItem*)cargo_data;
            if (item->getType() == LLAssetType::AT_SOUND)
            {
                *accept = ACCEPT_YES_SINGLE;

                if (drop)
                {
                    volobjp->setRexSoundID(item->getAssetUUID());
                    getState();
                }
            }
        }
        handled = TRUE;
    }

    return handled;
}
