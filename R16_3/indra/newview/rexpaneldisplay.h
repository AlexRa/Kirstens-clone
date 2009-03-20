/**
 * @file rexpaneldisplay.h
 * @brief RexPanelDisplay class header file
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef REXPANELDISPLAY_H
#define REXPANELDISPLAY_H

#include "rexpanelprim.h"

class LLViewerObject;
class LLVOVolume;

//! Panel for editing extended display properties of a prim.
class RexPanelDisplay : public RexPanelPrim
{
public:
    //! Constructor, called from LLFloaterTools's creation factory method
    RexPanelDisplay(const std::string& name);
    //! Destructor
    virtual ~RexPanelDisplay();

    //! Postbuild, hooks up UI callbacks
    virtual BOOL postBuild();

    static void onCommitIsVisible(LLUICtrl* ctrl, void* userdata);
    static void onCommitDrawType(LLUICtrl* ctrl, void* userdata);
    static void onCommitShowText(LLUICtrl* ctrl, void* userdata);
    static void onCommitCastShadows(LLUICtrl* ctrl, void* userdata);
    static void onCommitLightCastsShadows(LLUICtrl* ctrl, void* userdata);
    static void onCommitScaleMesh(LLUICtrl* ctrl, void* userdata);
    static void onCommitDrawDistance(LLUICtrl* ctrl, void* userdata);
    static void onCommitLODBias(LLUICtrl* ctrl, void* userdata);
    static void onCommitMeshID(LLUICtrl* ctrl, void* userdata);
    static void onCommitCollisionMeshID(LLUICtrl* ctrl, void* userdata);
    static void onCommitAnimationPackID(LLUICtrl* ctrl, void* userdata);
    static void onCommitAnimName(LLUICtrl* ctrl, void* userdata);
    static void onCommitAnimRate(LLUICtrl* ctrl, void* userdata);
    static void onCommitParticleScriptID(LLUICtrl* ctrl, void* userdata);
    static void onClickMeshEdit(void *data);

    //! Handles dragging inventory assets into panel controls
    BOOL handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 LLString& tooltip_msg);

    // Functions needed by LLFloaterMeshEdit
    S32 getMeshIndex(const std::string& MeshName);
    std::string getMeshName(const LLUUID& id);
    void setNewMesh(const LLUUID& id);

protected:
    //! Queries object to get values into controls
    virtual void getState();
    //! Activates or deactivates controls 
    virtual void enableCtrls(bool enabled);
    //! Gets lists of known mesh & particle & animationpack assets from inventory
    void getAssetNames();
    //! Gets combobox index related to selected object's mesh asset
    S32 getMeshIndex(LLVOVolume* volobjp);
    //! Gets combobox index related to selected object's animationpack asset
    S32 getAnimationPackIndex(LLVOVolume* volobjp);
    //! Gets combobox index related to selected object's collisionmesh asset
    S32 getCollisionMeshIndex(LLVOVolume* volobjp);
    //! Gets combobox index related to selected object's particle asset
    S32 getParticleScriptIndex(LLVOVolume* volobjp);

    //! List of mesh name/UUID pairs, queried from inventory
    std::vector<RexPanelSelection> mMeshNames;
    //! List of animationpack name/UUID pairs, queried from inventory
    std::vector<RexPanelSelection> mAnimationPackNames;
    //! List of particlescript name/UUID pairs, queried from inventory
    std::vector<RexPanelSelection> mParticleScriptNames;
};

#endif // REXPANELDISPLAY_H
