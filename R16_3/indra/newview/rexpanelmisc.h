/**
 * @file rexpanelmisc.h
 * @brief RexPanelMisc class header file
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef REXPANELMISC_H
#define REXPANELMISC_H

#include "rexpanelprim.h"

class LLViewerObject;
class LLVOVolume;

//! Panel for editing prim's misc extended properties.
class RexPanelMisc : public RexPanelPrim
{
public:
    //! Constructor, called from LLFloaterTools's creation factory method
	RexPanelMisc(const std::string& name);
    //! Destructor
	virtual ~RexPanelMisc();

    //! Postbuild, hooks up UI callbacks
	virtual BOOL postBuild();

    static void onCommitClassName(LLUICtrl* ctrl, void* userdata);
    static void onCommitSoundID(LLUICtrl* ctrl, void* userdata);
    static void onCommitSoundVolume(LLUICtrl* ctrl, void* userdata);
    static void onCommitSoundRadius(LLUICtrl* ctrl, void* userdata);
    static void onCommitSelectionPriority(LLUICtrl* ctrl, void* userdata);

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
    //! Gets lists of assets (currently sounds) used by Misc panel from inventory
    void getAssetNames();
    //! Gets combobox index related to selected object's sound asset
    S32 getSoundIndex(LLVOVolume* volobjp);

    //! List of sound name/UUID pairs, queried from inventory
    std::vector<RexPanelSelection> mSoundNames;
};

#endif // REXPANELMISC_H
