/** 
 * @file llprogressbar.cpp
 * @brief LLProgressBar class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2010, Linden Research, Inc.
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

#include "linden_common.h"

#include "llprogressbar.h"

#include "indra_constants.h"
#include "llmath.h"
#include "llgl.h"
#include "llui.h"
#include "llfontgl.h"
#include "lltimer.h"
#include "llglheaders.h"

#include "llfocusmgr.h"
#include "lluictrlfactory.h"

static LLDefaultChildRegistry::Register<LLProgressBar> r("progress_bar");

LLProgressBar::Params::Params()
:	image_bar("image_bar"),
	image_fill("image_fill"),
	color_bar("color_bar"),
	color_bg("color_bg")
{}


LLProgressBar::LLProgressBar(const LLProgressBar::Params& p) 
:	LLView(p),
	mImageBar(p.image_bar),
	mImageFill(p.image_fill),
	mColorBackground(p.color_bg()),
	mColorBar(p.color_bar()),
	mPercentDone(0.f)
{}

LLProgressBar::~LLProgressBar()
{
	gFocusMgr.releaseFocusIfNeeded( this );
}

void LLProgressBar::draw()
{
	static LLTimer timer;
	F32 alpha = getDrawContext().mAlpha;
	
	LLColor4 image_bar_color = mColorBackground.get();
	image_bar_color.setAlpha(alpha);
	mImageBar->draw(getLocalRect(), image_bar_color);

	alpha *= 0.5f + 0.5f*0.5f*(1.f + (F32)sin(3.f*timer.getElapsedTimeF32()));
	LLColor4 bar_color = mColorBar.get();
	bar_color.mV[VALPHA] *= alpha; // modulate alpha
	LLRect progress_rect = getLocalRect();
	progress_rect.mRight = llround(getRect().getWidth() * (mPercentDone / 100.f));
	mImageFill->draw(progress_rect, bar_color);
}

void LLProgressBar::setPercent(const F32 percent)
{
	mPercentDone = llclamp(percent, 0.f, 100.f);
}
