/**
 * @file rexpanelprim.h
 * @brief RexPanelPrim class header file
 *
 * Copyright (c) 2008 LudoCraft
 */

#ifndef REXPANELPRIM_H
#define REXPANELPRIM_H

#include "llpanel.h"

class LLViewerObject;

//! Structure for asset combobox selections; name & associated asset UUID
struct RexPanelSelection
{
	LLUUID mUUID;
    std::string mName;
};

//! Base class for various reX prim data editor panels
class RexPanelPrim : public LLPanel
{
public:
    //! Constructor
    RexPanelPrim(const std::string& name);
    //! Destructor
    virtual ~RexPanelPrim();

    //! Postbuild, hooks up UI callbacks
    virtual BOOL postBuild() = 0; 
    //! Deactivates controls
    virtual void clearCtrls();
    //! Refreshes selected object from selectionmanager, sets mObject non-zero if object is a valid prim
    virtual void refresh();

protected:
    //! Queries object to get values into controls
    virtual void getState() = 0;
    //! Activates or deactivates controls 
    virtual void enableCtrls(bool enabled) = 0;

    //! Pointer to currently selected object. Will be null if selection not valid (object not a prim, for example)
	LLViewerObject* mObject;
};

#endif // REXPANELPRIM_H