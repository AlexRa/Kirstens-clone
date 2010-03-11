/** 
 * @file llpanelplacestab.h
 * @brief Tabs interface for Side Bar "Places" panel
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

#ifndef LL_LLPANELAPPEARANCETAB_H
#define LL_LLPANELAPPEARANCETAB_H

#include "llpanel.h"

#include "llpanelappearance.h"

class LLPanelAppearanceTab : public LLPanel
{
public:
	LLPanelAppearanceTab(LLPanelAppearance *parent) : 
		LLPanel(),
		mParent(parent)
	{}
	virtual ~LLPanelAppearanceTab() {}

	virtual void onSearchEdit(const std::string& string) = 0;
	virtual void updateVerbs() = 0;		// Updates buttons at the bottom of Appearance panel
	virtual void onWear() = 0;
	virtual void onEdit() = 0;
	virtual void onNew() = 0;

	bool isTabVisible(); // Check if parent TabContainer is visible.


protected:
	LLPanelAppearance*		mParent;
};

#endif //LL_LLPANELAPPEARANCETAB_H
