/**
 * @file rexpanelmaterial.cpp
 * @brief RexPanelMaterial class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

#include <sstream>

// file include
#include "rexpanelmaterial.h"

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

#define MAX_VISIBLE_MATERIALS 12

RexPanelMaterial::RexPanelMaterial(const std::string& name)
	:	RexPanelPrim(name)
{
	setMouseOpaque(FALSE);
}

// virtual
RexPanelMaterial::~RexPanelMaterial()
{
	// Children all cleaned up by default view destructor.
}

// virtual
BOOL RexPanelMaterial::postBuild()
{	
	refresh();

    mComboBoxes.clear();
    for (unsigned i = 1; i <= MAX_VISIBLE_MATERIALS; ++i)
    {
        std::ostringstream ctrlName;
        ctrlName << "RexMaterialName" << i << " Ctrl";
        mComboBoxes.push_back(getChild<LLComboBox>(ctrlName.str()));
        childSetCommitCallback(ctrlName.str(), onCommitMaterialID, this);
    }

	return TRUE;
}

// virtual
void RexPanelMaterial::getState()
{
    if (!mObject) 
        return;

    LLVOVolume *volobjp = (LLVOVolume *)mObject;

    getAssetNames();

    for (unsigned i = 0; i < mComboBoxes.size(); ++i)
    {
        if (mComboBoxes[i])
        {
            mComboBoxes[i]->setValue(getMaterialIndex(volobjp, i));
        }
    }
}

// virtual
void RexPanelMaterial::enableCtrls(bool enabled)
{
    for (unsigned i = 1; i <= MAX_VISIBLE_MATERIALS; ++i)
    {
        bool enableThis = enabled;
        if (enableThis)
        {
            if (!mObject)
                enableThis = false;
            else
            {
                LLVOVolume *volobjp = (LLVOVolume *)mObject;
                if ((i > 1) && (i > volobjp->getRexNumMaterials()))
                    enableThis = false;
            }
        }

        std::ostringstream textName;
        textName << "RexMaterialName" << i;
        std::ostringstream ctrlName;
        ctrlName << "RexMaterialName" << i << " Ctrl";

        childSetEnabled(textName.str(), enableThis);
        childSetEnabled(ctrlName.str(), enableThis);
    }
}

// static
void RexPanelMaterial::onCommitMaterialID( LLUICtrl* ctrl, void* userdata )
{
	RexPanelMaterial* self = (RexPanelMaterial*) userdata;

    if (self->mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        LLComboBox* comboBox = (LLComboBox*)ctrl;

        S32 index = ctrl->getValue();
        if (index == -1) return; // Do not send "unknown"

        // Find out which combobox (and thus which submesh) was changed
        U32 subMeshIndex = 0;
        for (unsigned i = 0; i < self->mComboBoxes.size(); ++i)
        {
            if (comboBox == self->mComboBoxes[i])
            {
                subMeshIndex = i;
                break;
            }
        }

        RexMaterialDef newDef;
        newDef.mAssetID = LLUUID::null;

        if ((index >= 0) && (index < (S32)self->mMaterialNames.size()))
        {
            newDef.mAssetID = self->mMaterialNames[index].mUUID;
            newDef.mUseMaterialScript = self->mMaterialNames[index].mIsMaterialScript;
        }
        volobjp->setRexMaterialDef(subMeshIndex, newDef);

        self->getState();
    }
}

void RexPanelMaterial::getAssetNames()
{
    for (unsigned i = 0; i < mComboBoxes.size(); ++i)
    {
        LLComboBox* comboBox = mComboBoxes[i];
        if (comboBox)
        {
	        comboBox->removeall();
	        comboBox->add(LLString("<Unknown>"), -1);
        }
    }

	mMaterialNames.clear();

	// Add blank texture selection first
	{
		RexMaterialPanelSelection temp;
		temp.mUUID = LLUUID::null;
		temp.mName = "<Blank>";
        temp.mIsMaterialScript = false;
        for (unsigned i = 0; i < mComboBoxes.size(); ++i)
        {
            LLComboBox* comboBox = mComboBoxes[i];
            if (comboBox)
		        comboBox->add(temp.mName, (S32)mMaterialNames.size());
        }
		mMaterialNames.push_back(temp);
	}

	// Then add all texture/material assets to comboboxes
	{
		LLViewerInventoryCategory::cat_array_t cats;
		LLViewerInventoryItem::item_array_t items;
		LLIsType isType(LLAssetType::AT_TEXTURE);
		gInventory.collectDescendentsIf(LLUUID::null,
								cats,
								items,
								LLInventoryModel::INCLUDE_TRASH,
								isType);

		for (S32 i = 0; i < items.count(); i++)
		{
			LLInventoryItem* itemp = items[i];
			RexMaterialPanelSelection temp;
			temp.mUUID = itemp->getAssetUUID();
			temp.mName = itemp->getName();
            temp.mIsMaterialScript = false;
            for (unsigned j = 0; j < mComboBoxes.size(); ++j)
            {
                LLComboBox* comboBox = mComboBoxes[j];
                if (comboBox)
    		        comboBox->add(temp.mName, (S32)mMaterialNames.size());
            }
			mMaterialNames.push_back(temp);
		}
	}

    {
		LLViewerInventoryCategory::cat_array_t cats;
		LLViewerInventoryItem::item_array_t items;
		LLIsType isType(LLAssetType::AT_MATERIAL);
		gInventory.collectDescendentsIf(LLUUID::null,
								cats,
								items,
								LLInventoryModel::INCLUDE_TRASH,
								isType);

		for (S32 i = 0; i < items.count(); i++)
		{
			LLInventoryItem* itemp = items[i];
			RexMaterialPanelSelection temp;
			temp.mUUID = itemp->getAssetUUID();
			temp.mName = itemp->getName();
            temp.mIsMaterialScript = true;
            for (unsigned j = 0; j < mComboBoxes.size(); ++j)
            {
                LLComboBox* comboBox = mComboBoxes[j];
                if (comboBox)
    		        comboBox->add(temp.mName, (S32)mMaterialNames.size());
            }
			mMaterialNames.push_back(temp);
		}
	}

    for (unsigned i = 0; i < mComboBoxes.size(); ++i)
    {
        LLComboBox* comboBox = mComboBoxes[i];
        if (comboBox)
            comboBox->sortByName();
    }
}

S32 RexPanelMaterial::getMaterialIndex(LLVOVolume* volobjp, U32 index)
{
    const RexMaterialDef& matDef = volobjp->getRexMaterialDef(index);
	for (U32 i = 0; i < mMaterialNames.size(); ++i)
	{
		if ((mMaterialNames[i].mUUID == matDef.mAssetID) && (mMaterialNames[i].mIsMaterialScript == matDef.mUseMaterialScript))
		{
			return i;
		}
	}
    
    return -1;
}

BOOL RexPanelMaterial::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 LLString& tooltip_msg)
{
	BOOL handled = FALSE;

    if (!mObject)
        return handled;
    LLVOVolume *volobjp = (LLVOVolume *)mObject;

    for (unsigned i = 0; i < mComboBoxes.size(); ++i)
    {
        LLComboBox* material = mComboBoxes[i];

        if (material->getRect().pointInRect(x,y))
        {
            if ((cargo_type == DAD_TEXTURE) || (cargo_type == DAD_OGREASSET))
            {
                LLInventoryItem* item = (LLInventoryItem*)cargo_data;
                if (item->getType() == LLAssetType::AT_TEXTURE)
                {
                    *accept = ACCEPT_YES_SINGLE;
    
                    if (drop)
                    {
                        RexMaterialDef newDef;
                        newDef.mAssetID = item->getAssetUUID();
                        newDef.mUseMaterialScript = false;
                        volobjp->setRexMaterialDef(i, newDef);
                        getState();
                    }	
                }
    
                if (item->getType() == LLAssetType::AT_MATERIAL)
                {
                    *accept = ACCEPT_YES_SINGLE;
    
                    if (drop)
                    {
                        RexMaterialDef newDef;
                        newDef.mAssetID = item->getAssetUUID();
                        newDef.mUseMaterialScript = true;
                        volobjp->setRexMaterialDef(i, newDef);
                        getState();
                    }
                }
            }
        }
        handled = TRUE;
    }

    return handled;
}

