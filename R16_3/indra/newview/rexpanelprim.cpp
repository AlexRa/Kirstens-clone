/**
 * @file rexpanelprim.cpp
 * @brief RexPanelPrim class implementation
 *
 * Copyright (c) 2008 LudoCraft
 */

#include "llviewerprecompiledheaders.h"

// file include
#include "rexpanelprim.h"

#include "llselectmgr.h"
#include "llvovolume.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llfocusmgr.h"

RexPanelPrim::RexPanelPrim(const std::string& name)
	:	LLPanel(name)
{
	setMouseOpaque(FALSE);
}

// virtual
RexPanelPrim::~RexPanelPrim()
{
	// Children all cleaned up by default view destructor.
}

// virtual
void RexPanelPrim::clearCtrls()
{
    LLPanel::clearCtrls();
}

// virtual
void RexPanelPrim::refresh()
{	
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getFirstObject();
	mObject = objectp;

	if(!objectp)
	{
		//forfeit focus
		if (gFocusMgr.childHasKeyboardFocus(this))
		{
			gFocusMgr.setKeyboardFocus(NULL, NULL);
		}

		// Disable all input fields
		clearCtrls();
		return;
	}

	// Verify that we've selected only a single volume object and it is editable
	BOOL editable = objectp->permModify();
	BOOL single_volume = objectp->getPCode() == LL_PCODE_VOLUME && LLSelectMgr::getInstance()->selectionAllPCode( LL_PCODE_VOLUME )
		&& LLSelectMgr::getInstance()->getSelection()->getObjectCount() == 1;
	if ((!editable) || (!single_volume))
	{
		clearCtrls();
        mObject = NULL;
		return;
	}

    enableCtrls(true);
	getState();
}
