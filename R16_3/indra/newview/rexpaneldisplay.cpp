/**
 * @file rexpaneldisplay.cpp
 * @brief RexPanelDisplay class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

// file include
#include "rexpaneldisplay.h"

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

RexPanelDisplay::RexPanelDisplay(const std::string& name)
	:	RexPanelPrim(name)
{
}

// virtual
RexPanelDisplay::~RexPanelDisplay()
{
	// Children all cleaned up by default view destructor.
}

// virtual
BOOL RexPanelDisplay::postBuild()
{	
	refresh();

    childSetCommitCallback("RexDrawType Ctrl", onCommitDrawType, this);
    childSetCommitCallback("RexIsVisible Ctrl", onCommitIsVisible, this);
    childSetCommitCallback("RexShowText Ctrl", onCommitShowText, this);
    childSetCommitCallback("RexCastShadows Ctrl", onCommitCastShadows, this);
    childSetCommitCallback("RexLightCastsShadows Ctrl", onCommitLightCastsShadows, this);
    childSetCommitCallback("RexScaleMesh Ctrl", onCommitScaleMesh, this);
    childSetCommitCallback("RexDrawDistance Ctrl", onCommitDrawDistance, this);
    childSetCommitCallback("RexLODBias Ctrl", onCommitLODBias, this);
    childSetCommitCallback("RexMeshName Ctrl", onCommitMeshID, this);
    childSetCommitCallback("RexCollisionMeshName Ctrl", onCommitCollisionMeshID, this);
    childSetCommitCallback("RexAnimationPackName Ctrl", onCommitAnimationPackID, this);
    childSetCommitCallback("RexAnimName Ctrl", onCommitAnimName, this);
    childSetCommitCallback("RexAnimRate Ctrl", onCommitAnimRate, this);
    childSetCommitCallback("RexParticleScriptName Ctrl", onCommitParticleScriptID, this);

	childSetAction("button Mesh UVflip", onClickMeshEdit,this);

	return TRUE;
}

// virtual
void RexPanelDisplay::enableCtrls(bool enabled)
{
    childSetEnabled("RexDrawType", enabled);
    childSetEnabled("RexDrawType Ctrl", enabled);
    childSetEnabled("RexIsVisible Ctrl", enabled);
    childSetEnabled("RexShowText Ctrl", enabled);
    childSetEnabled("RexCastShadows Ctrl", enabled);
    childSetEnabled("RexLightCastsShadows Ctrl", enabled);
    childSetEnabled("RexScaleMesh Ctrl", enabled);
    childSetEnabled("RexDrawDistance Ctrl", enabled);
    childSetEnabled("RexLODBias Ctrl", enabled);
    childSetEnabled("RexMeshName", enabled);
    childSetEnabled("RexMeshName Ctrl", enabled);
    childSetEnabled("button Mesh UVflip", enabled);
    childSetEnabled("RexCollisionMeshName", enabled);
    childSetEnabled("RexCollisionMeshName Ctrl", enabled);
    childSetEnabled("RexAnimationPackName", enabled);
    childSetEnabled("RexAnimationPackName Ctrl", enabled);
    childSetEnabled("RexAnimName", enabled);
    childSetEnabled("RexAnimName Ctrl", enabled);
    childSetEnabled("RexAnimRate Ctrl", enabled);
    childSetEnabled("RexParticleScriptName", enabled);
    childSetEnabled("RexParticleScriptName Ctrl", enabled);
}

// virtual
void RexPanelDisplay::getState()
{
    if (!mObject) 
        return;

    LLVOVolume *volobjp = (LLVOVolume *)mObject;

    // enable uvflip edit button if primObject have mesh
    const LLUUID& meshId = volobjp->getRexMeshID();
    if(meshId.notNull())
        childSetEnabled("button Mesh UVflip",TRUE); 
    else
        childSetEnabled("button Mesh UVflip",FALSE); 

	getAssetNames();

    // Get the actual values
    childSetValue("RexDrawType Ctrl", volobjp->getRexDrawType());
    childSetValue("RexIsVisible Ctrl", volobjp->getRexIsVisible());
    childSetValue("RexShowText Ctrl", volobjp->getRexShowText());
    childSetValue("RexCastShadows Ctrl", volobjp->getRexCastShadows());
    childSetValue("RexLightCastsShadows Ctrl", volobjp->getRexLightCastsShadows());
    childSetValue("RexScaleMesh Ctrl", volobjp->getRexScaleMesh());
    childSetValue("RexDrawDistance Ctrl", volobjp->getRexDrawDistance());
    childSetValue("RexLODBias Ctrl", volobjp->getRexLOD());
    childSetValue("RexAnimName Ctrl", volobjp->getRexAnimationName());
    childSetValue("RexAnimRate Ctrl", volobjp->getRexAnimationRate());
    childSetValue("RexMeshName Ctrl", getMeshIndex(volobjp));
    childSetValue("RexCollisionMeshName Ctrl", getCollisionMeshIndex(volobjp));
    childSetValue("RexParticleScriptName Ctrl", getParticleScriptIndex(volobjp));
    childSetValue("RexAnimationPackName Ctrl", getAnimationPackIndex(volobjp));
}

void RexPanelDisplay::onCommitDrawType( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        int value = self->childGetValue("RexDrawType Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexDrawType((RexPrimData::RexDrawType)value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitIsVisible( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        BOOL value = self->childGetValue("RexIsVisible Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexIsVisible(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitShowText( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        BOOL value = self->childGetValue("RexShowText Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexShowText(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitCastShadows( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        BOOL value = self->childGetValue("RexCastShadows Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexCastShadows(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitLightCastsShadows( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        BOOL value = self->childGetValue("RexLightCastsShadows Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexLightCastsShadows(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitScaleMesh( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        BOOL value = self->childGetValue("RexScaleMesh Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexScaleMesh(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitDrawDistance( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        F32 value = self->childGetValue("RexDrawDistance Ctrl").asReal();
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexDrawDistance(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitLODBias( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        F32 value = self->childGetValue("RexLODBias Ctrl").asReal();
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexLOD(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitMeshID( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;

        S32 index = self->childGetValue("RexMeshName Ctrl");
        if (index == -1) return; // Do not send "unknown"

        LLUUID id = LLUUID::null;
        if ((index >= 0) && (index < (S32)self->mMeshNames.size()))
        {
            id = self->mMeshNames[index].mUUID;
        }

        if (id != LLUUID::null)
        {
            volobjp->setRexMeshID(id, true); // Suppress sending this update
            volobjp->setRexDrawType(RexPrimData::DRAWTYPE_MESH); // Now send this
        }
        else
        {
            volobjp->setRexMeshID(id);
        }

        self->getState();
    }
}

void RexPanelDisplay::onCommitCollisionMeshID( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;

        S32 index = self->childGetValue("RexCollisionMeshName Ctrl");
        if (index == -1) return; // Do not send "unknown"

        LLUUID id = LLUUID::null;
        if ((index >= 0) && (index < (S32)self->mMeshNames.size()))
        {
            id = self->mMeshNames[index].mUUID;
        }
        volobjp->setRexCollisionMeshID(id);

        self->getState();
    }
}

void RexPanelDisplay::onCommitAnimationPackID( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;

        S32 index = self->childGetValue("RexAnimationPackName Ctrl");
        if (index == -1) return; // Do not send "unknown"

        LLUUID id = LLUUID::null;
        if ((index >= 0) && (index < (S32)self->mAnimationPackNames.size()))
        {
            id = self->mAnimationPackNames[index].mUUID;
        }
        volobjp->setRexAnimationPackID(id);

        self->getState();
    }
}

void RexPanelDisplay::onCommitAnimName( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        LLString value = self->childGetValue("RexAnimName Ctrl");
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexAnimationName(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitAnimRate( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*) userdata;

    if (self->mObject)
    {
        F32 value = self->childGetValue("RexAnimRate Ctrl").asReal();
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;
        volobjp->setRexAnimationRate(value);
        self->getState();
    }
}

void RexPanelDisplay::onCommitParticleScriptID( LLUICtrl* ctrl, void* userdata )
{
	RexPanelDisplay* self = (RexPanelDisplay*)userdata;

    if (self->mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)self->mObject;

        S32 index = self->childGetValue("RexParticleScriptName Ctrl");
        if (index == -1) return; // Do not send "unknown"

        LLUUID id = LLUUID::null;
        if ((index >= 0) && (index < (S32)self->mParticleScriptNames.size()))
        {
            id = self->mParticleScriptNames[index].mUUID;
        }
        volobjp->setRexParticleScriptID(id);

        self->getState();
    }
}

void RexPanelDisplay::onClickMeshEdit(void *data) 
{
	RexPanelDisplay *self = (RexPanelDisplay *)data;

	LLViewerObject* objectp = self->mObject;
	if (!objectp || (objectp->getPCode() != LL_PCODE_VOLUME))
	{
		return;
	}	
	LLVOVolume *volobjp = (LLVOVolume *)objectp;

	const LLUUID& meshId = volobjp->getRexMeshID();

	// open mesh editor window
	LLFloaterMeshEditor::CreateFloater(meshId, self->getMeshName(meshId) );	
}

void RexPanelDisplay::getAssetNames()
{
	LLComboBox* comboBox = getChild<LLComboBox>("RexMeshName Ctrl");
	LLComboBox* comboBox2 = getChild<LLComboBox>("RexCollisionMeshName Ctrl");
	if (!comboBox) return;
	if (!comboBox2) return;

	comboBox->removeall();
	comboBox->add(LLString("<Unknown>"), -1);
	comboBox2->removeall();
	comboBox2->add(LLString("<Unknown>"), -1);
	mMeshNames.clear();

	// Add null (none) selection to meshes
	{
		RexPanelSelection temp;
		temp.mUUID = LLUUID::null;
		temp.mName = "<None>";
		comboBox->add(temp.mName, (S32)mMeshNames.size());
        comboBox2->add(temp.mName, (S32)mMeshNames.size());
		mMeshNames.push_back(temp);
	}

	// Add all mesh assets to comboboxes
	{
		LLViewerInventoryCategory::cat_array_t cats;
		LLViewerInventoryItem::item_array_t items;
		LLIsType isType(LLAssetType::AT_MESH);
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
            comboBox->add(temp.mName, (S32)mMeshNames.size());
			comboBox2->add(temp.mName, (S32)mMeshNames.size());
            mMeshNames.push_back(temp);
		}
	}

    comboBox->sortByName();
    comboBox2->sortByName();

	comboBox = getChild<LLComboBox>("RexParticleScriptName Ctrl");
	if (!comboBox) return;

	comboBox->removeall();
	comboBox->add(LLString("<Unknown>"), -1);
	mParticleScriptNames.clear();

	// Add null particlescript selection first
	{
		RexPanelSelection temp;
		temp.mUUID = LLUUID::null;
		temp.mName = "<None>";
		comboBox->add(temp.mName, (S32)mParticleScriptNames.size());
		mParticleScriptNames.push_back(temp);
	}

	// Then add all particlescript assets to combobox
	{
		LLViewerInventoryCategory::cat_array_t cats;
		LLViewerInventoryItem::item_array_t items;
		LLIsType isType(LLAssetType::AT_PARTICLE_SCRIPT);
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
			comboBox->add(temp.mName, (S32)mParticleScriptNames.size());
			mParticleScriptNames.push_back(temp);
		}
	}

    comboBox->sortByName();

	comboBox = getChild<LLComboBox>("RexAnimationPackName Ctrl");
	if (!comboBox) return;

	comboBox->removeall();
	comboBox->add(LLString("<Unknown>"), -1);
	mAnimationPackNames.clear();

	// Add null animationpack selection first
	{
		RexPanelSelection temp;
		temp.mUUID = LLUUID::null;
		temp.mName = "<None>";
		comboBox->add(temp.mName, (S32)mAnimationPackNames.size());
		mAnimationPackNames.push_back(temp);
	}

	// Then add all ogreskeleton assets to combobox
	{
		LLViewerInventoryCategory::cat_array_t cats;
		LLViewerInventoryItem::item_array_t items;
      LLIsType isType(LLAssetType::AT_SKELETON);
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
			comboBox->add(temp.mName, (S32)mAnimationPackNames.size());
			mAnimationPackNames.push_back(temp);
		}
	}

    comboBox->sortByName();
}

S32 RexPanelDisplay::getMeshIndex(LLVOVolume *volobjp)
{
	const LLUUID& meshId = volobjp->getRexMeshID();

	for (U32 i = 0; i < mMeshNames.size(); ++i)
	{
		if (mMeshNames[i].mUUID == meshId)
		{
			return i;
		}
	}

	return -1;
}

S32 RexPanelDisplay::getCollisionMeshIndex(LLVOVolume *volobjp)
{
	const LLUUID& meshId = volobjp->getRexCollisionMeshID();

	for (U32 i = 0; i < mMeshNames.size(); ++i)
	{
		if (mMeshNames[i].mUUID == meshId)
		{
			return i;
		}
	}

	return -1;
}

S32 RexPanelDisplay::getParticleScriptIndex(LLVOVolume *volobjp)
{
	const LLUUID& particleScriptId = volobjp->getRexParticleScriptID();

	for (U32 i = 0; i < mParticleScriptNames.size(); ++i)
	{
		if (mParticleScriptNames[i].mUUID == particleScriptId)
		{
			return i;
		}
	}

	return -1;
}

S32 RexPanelDisplay::getAnimationPackIndex(LLVOVolume *volobjp)
{
	const LLUUID& animationPackId = volobjp->getRexAnimationPackID();

	for (U32 i = 0; i < mAnimationPackNames.size(); ++i)
	{
		if (mAnimationPackNames[i].mUUID == animationPackId)
		{
			return i;
		}
	}

	return -1;
}

BOOL RexPanelDisplay::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 LLString& tooltip_msg)
{
	BOOL handled = FALSE;

    LLComboBox*	mesh = getChild<LLComboBox>(  "RexMeshName Ctrl");
    if (!mesh) return handled;
    LLComboBox* collisionMesh = getChild<LLComboBox>(  "RexCollisionMeshName Ctrl"); 
    if (!collisionMesh) return handled;
    LLComboBox* animation = getChild<LLComboBox>(  "RexAnimationPackName Ctrl"); 
    if (!animation) return handled;
    LLComboBox* particle = getChild<LLComboBox>(  "RexParticleScriptName Ctrl"); 
    if (!particle) return handled;

    LLViewerObject* objectp = mObject;
    if (!objectp) return handled;
    LLVOVolume *volobjp = (LLVOVolume *)objectp;

    if (mesh->getRect().pointInRect(x,y))
    {
        if ((cargo_type == DAD_OBJECT) || (cargo_type == DAD_OGREASSET))
        {
            LLInventoryItem* item = (LLInventoryItem*)cargo_data;
            if (item->getType() == LLAssetType::AT_MESH)
            {
                *accept = ACCEPT_YES_SINGLE;

                if (drop)
                {
                    volobjp->setRexMeshID(item->getAssetUUID(), true); // Suppress sending this update
                    volobjp->setRexDrawType(RexPrimData::DRAWTYPE_MESH); // Now send this
                    getState();
                }
            }
        }
        handled = TRUE;
    }

    if (collisionMesh->getRect().pointInRect(x,y))
    {
        if ((cargo_type == DAD_OBJECT) || (cargo_type == DAD_OGREASSET))
        {
            LLInventoryItem* item = (LLInventoryItem*)cargo_data;
            if (item->getType() == LLAssetType::AT_MESH)
            {
                *accept = ACCEPT_YES_SINGLE;

                if (drop)
                {
                    LLVOVolume *volobjp = (LLVOVolume *)objectp;
                    volobjp->setRexCollisionMeshID(item->getAssetUUID());
                    getState();
                }
            }
        }
        handled = TRUE;
    }

    if (particle->getRect().pointInRect(x,y))
    {
        if ((cargo_type == DAD_OBJECT) || (cargo_type == DAD_OGREASSET))
        {
            LLInventoryItem* item = (LLInventoryItem*)cargo_data;
            if (item->getType() == LLAssetType::AT_PARTICLE_SCRIPT)
            {
                *accept = ACCEPT_YES_SINGLE;

                if (drop)
                {
                    volobjp->setRexParticleScriptID(item->getAssetUUID());
                    getState();
                }
            }
        }
        handled = TRUE;
    }

    if (animation->getRect().pointInRect(x,y))
    {
        if ((cargo_type == DAD_OBJECT) || (cargo_type == DAD_OGREASSET))
        {
            LLInventoryItem* item = (LLInventoryItem*)cargo_data;
            if (item->getType() == LLAssetType::AT_SKELETON)
            {
                *accept = ACCEPT_YES_SINGLE;

                if (drop)
                {
                    volobjp->setRexAnimationPackID(item->getAssetUUID());
                    getState();
                }
            }
        }
        handled = TRUE;
    }

    return handled;
}

S32 RexPanelDisplay::getMeshIndex(const std::string& MeshName)
{
	for(S32 index = 0; (index < (S32)mMeshNames.size()); index++)
	{
		if (mMeshNames[index].mName == MeshName)
		{
			return index;
		}
	}
	return -1;
}

std::string RexPanelDisplay::getMeshName(const LLUUID& id)
{
	for(S32 index = 0; (index < (S32)mMeshNames.size()); index++)
	{
		if (mMeshNames[index].mUUID == id)
			{
				return mMeshNames[index].mName;
			}
	}
	return "No name";
}

void RexPanelDisplay::setNewMesh(const LLUUID& id)
{
    if (mObject)
    {
        LLVOVolume *volobjp = (LLVOVolume *)mObject;

        if (id != LLUUID::null)
        {
            volobjp->setRexMeshID(id, true); // Suppress sending this update
            volobjp->setRexDrawType(RexPrimData::DRAWTYPE_MESH); // Now send this
        }
        else
        {
            volobjp->setRexMeshID(id);
        }

        getState();
    }
}
