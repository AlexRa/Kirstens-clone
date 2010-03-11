/** 
 * @file llscrollcontainer.h
 * @brief LLScrollContainer class header file.
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

#ifndef LL_LLSCROLLCONTAINER_H
#define LL_LLSCROLLCONTAINER_H

#include "lluictrl.h"
#ifndef LL_V4COLOR_H
#include "v4color.h"
#endif
#include "stdenums.h"
#include "llcoord.h"
#include "llscrollbar.h"


class LLViewBorder;
class LLUICtrlFactory;

/*****************************************************************************
 * 
 * A decorator view class meant to encapsulate a clipped region which is
 * scrollable. It automatically takes care of pixel perfect scrolling
 * and cliipping, as well as turning the scrollbars on or off based on
 * the width and height of the view you're scrolling.
 *
 *****************************************************************************/

struct ScrollContainerRegistry : public LLChildRegistry<ScrollContainerRegistry>
{};

class LLScrollContainer : public LLUICtrl
{
public:
	// Note: vertical comes before horizontal because vertical
	// scrollbars have priority for mouse and keyboard events.
	enum SCROLL_ORIENTATION { VERTICAL, HORIZONTAL, SCROLLBAR_COUNT };

	struct Params : public LLInitParam::Block<Params, LLUICtrl::Params>
	{
		Optional<bool>		is_opaque,
							reserve_scroll_corner,
							border_visible,
							hide_scrollbar;
		Optional<F32>		min_auto_scroll_rate,
							max_auto_scroll_rate;
		Optional<LLUIColor>	bg_color;
		Optional<LLScrollbar::callback_t> scroll_callback;
		
		Params();
	};

	// my valid children are stored in this registry
 	typedef ScrollContainerRegistry child_registry_t;

protected:
	LLScrollContainer(const Params&);
	friend class LLUICtrlFactory;
public:
	virtual ~LLScrollContainer( void );

	virtual void 	setValue(const LLSD& value) { mInnerRect.setValue(value); }

	void			setBorderVisible( BOOL b );

	void			scrollToShowRect( const LLRect& rect, const LLRect& constraint);
	void			scrollToShowRect( const LLRect& rect) { scrollToShowRect(rect, LLRect(0, mInnerRect.getHeight(), mInnerRect.getWidth(), 0)); }

	void			setReserveScrollCorner( BOOL b ) { mReserveScrollCorner = b; }
	LLRect			getVisibleContentRect();
	LLRect			getContentWindowRect();
	const LLRect&	getScrolledViewRect() const { return mScrolledView ? mScrolledView->getRect() : LLRect::null; }
	void			pageUp(S32 overlap = 0);
	void			pageDown(S32 overlap = 0);
	void			goToTop();
	void			goToBottom();
	bool			isAtTop() { return mScrollbar[VERTICAL]->isAtBeginning(); }
	bool			isAtBottom() { return mScrollbar[VERTICAL]->isAtEnd(); }
	S32				getBorderWidth() const;

	// LLView functionality
	virtual void	reshape(S32 width, S32 height, BOOL called_from_parent = TRUE);
	virtual BOOL	handleKeyHere(KEY key, MASK mask);
	virtual BOOL	handleScrollWheel( S32 x, S32 y, S32 clicks );
	virtual BOOL	handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop,
								   EDragAndDropType cargo_type,
								   void* cargo_data,
								   EAcceptance* accept,
								   std::string& tooltip_msg);

	virtual void	draw();
	virtual bool	addChild(LLView* view, S32 tab_group = 0);
	
	bool autoScroll(S32 x, S32 y);

private:
	// internal scrollbar handlers
	virtual void scrollHorizontal( S32 new_pos );
	virtual void scrollVertical( S32 new_pos );
	void updateScroll();
	void calcVisibleSize( S32 *visible_width, S32 *visible_height, BOOL* show_h_scrollbar, BOOL* show_v_scrollbar ) const;

	LLScrollbar* mScrollbar[SCROLLBAR_COUNT];
	LLView*		mScrolledView;
	S32			mSize;
	BOOL		mIsOpaque;
	LLUIColor	mBackgroundColor;
	LLRect		mInnerRect;
	LLViewBorder* mBorder;
	BOOL		mReserveScrollCorner;
	BOOL		mAutoScrolling;
	F32			mAutoScrollRate;
	F32			mMinAutoScrollRate;
	F32			mMaxAutoScrollRate;
	bool		mHideScrollbar;
};


#endif // LL_LLSCROLLCONTAINER_H
