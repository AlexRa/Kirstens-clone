/**
 * @file llteleporthistorystorage.h
 * @brief Saving/restoring of teleport history.
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

#ifndef LL_LLTELEPORTHISTORYSTORAGE_H
#define LL_LLTELEPORTHISTORYSTORAGE_H

#include <vector>

#include "llsingleton.h"
#include "lldate.h"
#include "v3dmath.h"

class LLSD;

/**
 * An item of the teleport history, stored in file
 *
 * Contains the location's global coordinates, title and date.
 */
class LLTeleportHistoryPersistentItem
{
public:
	LLTeleportHistoryPersistentItem()
	{}

	LLTeleportHistoryPersistentItem(const std::string title, const LLVector3d& global_pos)
		: mTitle(title), mGlobalPos(global_pos), mDate(LLDate::now())
	{}

	LLTeleportHistoryPersistentItem(const std::string title, const LLVector3d& global_pos, const LLDate& date)
		: mTitle(title), mGlobalPos(global_pos), mDate(date)
	{}

	LLTeleportHistoryPersistentItem(const LLSD& val);
	LLSD toLLSD() const;

	std::string	mTitle;
	LLVector3d	mGlobalPos;
	LLDate		mDate;
};

/**
 * Persistent teleport history.
 *
 */
class LLTeleportHistoryStorage: public LLSingleton<LLTeleportHistoryStorage>
{
	LOG_CLASS(LLTeleportHistoryStorage);

public:

	typedef std::vector<LLTeleportHistoryPersistentItem> slurl_list_t;

	// removed_index is index of removed item, which replaced by more recent
	typedef boost::function<void(S32 removed_index)>		history_callback_t;
	typedef boost::signals2::signal<void(S32 removed_index)>	history_signal_t;

	LLTeleportHistoryStorage();
	~LLTeleportHistoryStorage();

	/**
	 * @return history items.
	 */
	const slurl_list_t& getItems() const { return mItems; }
	void			purgeItems();

	void addItem(const std::string title, const LLVector3d& global_pos);
	void addItem(const std::string title, const LLVector3d& global_pos, const LLDate& date);

	void removeItem(S32 idx);

	void save();
	void load();

	void dump() const;

	/**
	 * Set a callback to be called upon history changes.
	 * 
	 * Multiple callbacks can be set.
	 */
	boost::signals2::connection	setHistoryChangedCallback(history_callback_t cb);

	/**
	 * Go to specific item in the history.
	 * 
	 * The item is specified by its index (starting from 0).
	 */
	void					goToItem(S32 idx);

private:

	void onTeleportHistoryChange();
	bool compareByTitleAndGlobalPos(const LLTeleportHistoryPersistentItem& a, const LLTeleportHistoryPersistentItem& b);

	slurl_list_t	mItems;
	std::string	mFilename;

	/**
	 * Signal emitted when the history gets changed.
	 * 
	 * Invokes callbacks set with setHistoryChangedCallback().
	 */
	history_signal_t		mHistoryChangedSignal;
};

#endif //LL_LLTELEPORTHISTORYSTORAGE_H
