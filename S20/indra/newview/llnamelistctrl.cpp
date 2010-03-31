/** 
 * @file llnamelistctrl.cpp
 * @brief A list of names, automatically refreshed from name cache.
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 * 
 */

#include "llviewerprecompiledheaders.h"

#include "llnamelistctrl.h"

#include <boost/tokenizer.hpp>

#include "llcachename.h"
#include "llfloaterreg.h"
#include "llinventory.h"
#include "llscrolllistitem.h"
#include "llscrolllistcell.h"
#include "llscrolllistcolumn.h"
#include "llsdparam.h"
#include "lltooltip.h"

static LLDefaultChildRegistry::Register<LLNameListCtrl> r("name_list");

static const S32 info_icon_size = 16;

void LLNameListCtrl::NameTypeNames::declareValues()
{
	declare("INDIVIDUAL", LLNameListCtrl::INDIVIDUAL);
	declare("GROUP", LLNameListCtrl::GROUP);
	declare("SPECIAL", LLNameListCtrl::SPECIAL);
}

LLNameListCtrl::Params::Params()
:	name_column(""),
	allow_calling_card_drop("allow_calling_card_drop", false)
{
	name = "name_list";
}

LLNameListCtrl::LLNameListCtrl(const LLNameListCtrl::Params& p)
:	LLScrollListCtrl(p),
	mNameColumnIndex(p.name_column.column_index),
	mNameColumn(p.name_column.column_name),
	mAllowCallingCardDrop(p.allow_calling_card_drop)
{}

// public
void LLNameListCtrl::addNameItem(const LLUUID& agent_id, EAddPosition pos,
								 BOOL enabled, std::string& suffix)
{
	//llinfos << "LLNameListCtrl::addNameItem " << agent_id << llendl;

	NameItem item;
	item.value = agent_id;
	item.enabled = enabled;
	item.target = INDIVIDUAL;

	addNameItemRow(item, pos);
}

// virtual, public
BOOL LLNameListCtrl::handleDragAndDrop( 
		S32 x, S32 y, MASK mask,
		BOOL drop,
		EDragAndDropType cargo_type, void *cargo_data, 
		EAcceptance *accept,
		std::string& tooltip_msg)
{
	if (!mAllowCallingCardDrop)
	{
		return FALSE;
	}

	BOOL handled = FALSE;

	if (cargo_type == DAD_CALLINGCARD)
	{
		if (drop)
		{
			LLInventoryItem* item = (LLInventoryItem *)cargo_data;
			addNameItem(item->getCreatorUUID());
		}

		*accept = ACCEPT_YES_MULTI;
	}
	else
	{
		*accept = ACCEPT_NO;
		if (tooltip_msg.empty())
		{
			if (!getToolTip().empty())
			{
				tooltip_msg = getToolTip();
			}
			else
			{
				// backwards compatable English tooltip (should be overridden in xml)
				tooltip_msg.assign("Drag a calling card here\nto add a resident.");
			}
		}
	}

	handled = TRUE;
	lldebugst(LLERR_USER_INPUT) << "dragAndDrop handled by LLNameListCtrl " << getName() << llendl;

	return handled;
}

void LLNameListCtrl::showInspector(const LLUUID& avatar_id, bool is_group)
{
	if (is_group)
		LLFloaterReg::showInstance("inspect_group", LLSD().with("group_id", avatar_id));
	else
		LLFloaterReg::showInstance("inspect_avatar", LLSD().with("avatar_id", avatar_id));
}

void	LLNameListCtrl::mouseOverHighlightNthItem( S32 target_index )
{
	S32 cur_index = getHighlightedItemInx();
	if (cur_index != target_index)
	{
		if(0 <= cur_index && cur_index < (S32)getItemList().size())
		{
			LLScrollListItem* item = getItemList()[cur_index];
			LLScrollListText* cell = dynamic_cast<LLScrollListText*>(item->getColumn(mNameColumnIndex));
			if(cell)
				cell->setTextWidth(cell->getTextWidth() + info_icon_size);
		}
		if(target_index != -1)
		{
			LLScrollListItem* item = getItemList()[target_index];
			LLScrollListText* cell = dynamic_cast<LLScrollListText*>(item->getColumn(mNameColumnIndex));
			if(cell)
				cell->setTextWidth(cell->getTextWidth() - info_icon_size);
		}
	}

	LLScrollListCtrl::mouseOverHighlightNthItem(target_index);
}

//virtual
BOOL LLNameListCtrl::handleToolTip(S32 x, S32 y, MASK mask)
{
	BOOL handled = FALSE;
	S32 column_index = getColumnIndexFromOffset(x);
	LLScrollListItem* hit_item = hitItem(x, y);
	if (hit_item
		&& column_index == mNameColumnIndex)
	{
		// ...this is the column with the avatar name
		LLUUID avatar_id = hit_item->getUUID();
		if (avatar_id.notNull())
		{
			// ...valid avatar id

			LLScrollListCell* hit_cell = hit_item->getColumn(column_index);
			if (hit_cell)
			{
				S32 row_index = getItemIndex(hit_item);
				LLRect cell_rect = getCellRect(row_index, column_index);
				// Convert rect local to screen coordinates
				LLRect sticky_rect;
				localRectToScreen(cell_rect, &sticky_rect);

				// Spawn at right side of cell
				LLPointer<LLUIImage> icon = LLUI::getUIImage("Info_Small");
				LLCoordGL pos( sticky_rect.mRight - info_icon_size, sticky_rect.mTop - (sticky_rect.getHeight() - icon->getHeight())/2 );

				// Should we show a group or an avatar inspector?
				bool is_group = hit_item->getValue()["is_group"].asBoolean();

				LLToolTip::Params params;
				params.background_visible( false );
				params.click_callback( boost::bind(&LLNameListCtrl::showInspector, this, avatar_id, is_group) );
				params.delay_time(0.0f);		// spawn instantly on hover
				params.image( icon );
				params.message("");
				params.padding(0);
				params.pos(pos);
				params.sticky_rect(sticky_rect);

				LLToolTipMgr::getInstance()->show(params);
				handled = TRUE;
			}
		}
	}
	if (!handled)
	{
		handled = LLScrollListCtrl::handleToolTip(x, y, mask);
	}
	return handled;
}

// public
void LLNameListCtrl::addGroupNameItem(const LLUUID& group_id, EAddPosition pos,
									  BOOL enabled)
{
	NameItem item;
	item.value = group_id;
	item.enabled = enabled;
	item.target = GROUP;

	addNameItemRow(item, pos);
}

// public
void LLNameListCtrl::addGroupNameItem(LLNameListCtrl::NameItem& item, EAddPosition pos)
{
	item.target = GROUP;
	addNameItemRow(item, pos);
}

void LLNameListCtrl::addNameItem(LLNameListCtrl::NameItem& item, EAddPosition pos)
{
	item.target = INDIVIDUAL;
	addNameItemRow(item, pos);
}

LLScrollListItem* LLNameListCtrl::addElement(const LLSD& element, EAddPosition pos, void* userdata)
{
	LLNameListCtrl::NameItem item_params;
	LLParamSDParser::instance().readSD(element, item_params);
	item_params.userdata = userdata;
	return addNameItemRow(item_params, pos);
}


LLScrollListItem* LLNameListCtrl::addNameItemRow(
	const LLNameListCtrl::NameItem& name_item,
	EAddPosition pos,
	std::string& suffix)
{
	LLUUID id = name_item.value().asUUID();
	LLNameListItem* item = NULL;

	// Store item type so that we can invoke the proper inspector.
	// *TODO Vadim: Is there a more proper way of storing additional item data?
	{
		LLNameListCtrl::NameItem item_p(name_item);
		item_p.value = LLSD().with("uuid", id).with("is_group", name_item.target() == GROUP);
		item = new LLNameListItem(item_p);
		LLScrollListCtrl::addRow(item, item_p, pos);
	}

	if (!item) return NULL;

	// use supplied name by default
	std::string fullname = name_item.name;
	switch(name_item.target)
	{
	case GROUP:
		gCacheName->getGroupName(id, fullname);
		// fullname will be "nobody" if group not found
		break;
	case SPECIAL:
		// just use supplied name
		break;
	case INDIVIDUAL:
		{
			std::string name;
			if (gCacheName->getFullName(id, name))
			{
				fullname = name;
			}
			break;
		}
	default:
		break;
	}
	
	// Append optional suffix.
	if (!suffix.empty())
	{
		fullname.append(suffix);
	}

	LLScrollListCell* cell = item->getColumn(mNameColumnIndex);
	if (cell)
	{
		cell->setValue(fullname);
	}

	dirtyColumns();

	// this column is resizable
	LLScrollListColumn* columnp = getColumn(mNameColumnIndex);
	if (columnp && columnp->mHeader)
	{
		columnp->mHeader->setHasResizableElement(TRUE);
	}

	return item;
}

// public
void LLNameListCtrl::removeNameItem(const LLUUID& agent_id)
{
	// Find the item specified with agent_id.
	S32 idx = -1;
	for (item_list::iterator it = getItemList().begin(); it != getItemList().end(); it++)
	{
		LLScrollListItem* item = *it;
		if (item->getUUID() == agent_id)
		{
			idx = getItemIndex(item);
			break;
		}
	}

	// Remove it.
	if (idx >= 0)
	{
		selectNthItem(idx); // not sure whether this is needed, taken from previous implementation
		deleteSingleItem(idx);
	}
}

// public
void LLNameListCtrl::refresh(const LLUUID& id, const std::string& first, 
							 const std::string& last, BOOL is_group)
{
	//llinfos << "LLNameListCtrl::refresh " << id << " '" << first << " "
	//	<< last << "'" << llendl;

	std::string fullname;
	if (!is_group)
	{
		fullname = first + " " + last;
	}
	else
	{
		fullname = first;
	}

	// TODO: scan items for that ID, fix if necessary
	item_list::iterator iter;
	for (iter = getItemList().begin(); iter != getItemList().end(); iter++)
	{
		LLScrollListItem* item = *iter;
		if (item->getUUID() == id)
		{
			LLScrollListCell* cell = item->getColumn(mNameColumnIndex);
			if (cell)
			{
				cell->setValue(fullname);
			}
		}
	}

	dirtyColumns();
}


// static
void LLNameListCtrl::refreshAll(const LLUUID& id, const std::string& first,
								const std::string& last, BOOL is_group)
{
	LLInstanceTrackerScopedGuard guard;
	LLInstanceTracker<LLNameListCtrl>::instance_iter it;
	for (it = guard.beginInstances(); it != guard.endInstances(); ++it)
	{
		LLNameListCtrl& ctrl = *it;
		ctrl.refresh(id, first, last, is_group);
	}
}

void LLNameListCtrl::updateColumns()
{
	LLScrollListCtrl::updateColumns();

	if (!mNameColumn.empty())
	{
		LLScrollListColumn* name_column = getColumn(mNameColumn);
		if (name_column)
		{
			mNameColumnIndex = name_column->mIndex;
		}
	}
}
