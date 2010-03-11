/** 
 * @file llmousehandler.cpp
 * @brief LLMouseHandler class implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2010, Linden Research, Inc.
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

#include "llmousehandler.h"

//virtual
BOOL LLMouseHandler::handleAnyMouseClick(S32 x, S32 y, MASK mask, EClickType clicktype, BOOL down)
{
	BOOL handled = FALSE;
	if (down)
	{
		switch (clicktype)
		{
		case CLICK_LEFT: handled = handleMouseDown(x, y, mask); break;
		case CLICK_RIGHT: handled = handleRightMouseDown(x, y, mask); break;
		case CLICK_MIDDLE: handled = handleMiddleMouseDown(x, y, mask); break;
		case CLICK_DOUBLELEFT: handled = handleDoubleClick(x, y, mask); break;
		default:
			llwarns << "Unhandled enum." << llendl;
		}
	}
	else
	{
		switch (clicktype)
		{
		case CLICK_LEFT: handled = handleMouseUp(x, y, mask); break;
		case CLICK_RIGHT: handled = handleRightMouseUp(x, y, mask); break;
		case CLICK_MIDDLE: handled = handleMiddleMouseUp(x, y, mask); break;
		case CLICK_DOUBLELEFT: handled = handleDoubleClick(x, y, mask); break;
		default:
			llwarns << "Unhandled enum." << llendl;
		}
	}
	return handled;
}
