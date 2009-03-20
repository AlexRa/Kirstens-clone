/**
 * @file rexpanelmaterial.h
 * @brief RexPanelMaterial class header file
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef REXPANELMATERIAL_H
#define REXPANELMATERIAL_H

#include <vector>

#include "rexpanelprim.h"

class LLViewerObject;
class LLVOVolume;
class LLComboBox;

//! Structure for material/texture asset combobox selections
struct RexMaterialPanelSelection
{
	LLUUID mUUID;
    std::string mName;
    bool mIsMaterialScript;
};

//! Panel for selecting materials (textures or materialscripts) for mesh/prim.
class RexPanelMaterial : public RexPanelPrim
{
public:
    //! Constructor, called from LLFloaterTools's creation factory method
	RexPanelMaterial(const std::string& name);
    //! Destructor
	virtual ~RexPanelMaterial();

    //! Postbuild, hooks up UI callbacks
	virtual BOOL postBuild();

    static void onCommitMaterialID(LLUICtrl* ctrl, void* userdata);

    //! Handles dragging inventory assets into panel controls
    BOOL handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
									 EDragAndDropType cargo_type,
									 void* cargo_data,
									 EAcceptance* accept,
									 LLString& tooltip_msg);

protected:
    //! Queries object to get values into controls
    virtual void getState();
    //! Activates or deactivates controls 
    virtual void enableCtrls(bool enabled);

    //! Gets lists of known texture & materialscript assets from inventory
    void getAssetNames();
    //! Gets combobox index related to selected object's material asset
    S32 getMaterialIndex(LLVOVolume* volobjp, U32 index);

    //! List of material selector comboboxes contained in the panel
    std::vector<LLComboBox*> mComboBoxes;
    //! List of mesh name/UUID pairs, queried from inventory
    std::vector<RexMaterialPanelSelection> mMaterialNames;
};

#endif // REXPANELMATERIAL_H
