/** 
 * @file llinventoryobserver.cpp
 * @brief Implementation of the inventory observers used to track agent inventory.
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

#include "llviewerprecompiledheaders.h"

#include "llinventoryobserver.h"

#include "llassetstorage.h"
#include "llcrc.h"
#include "lldir.h"
#include "llsys.h"
#include "llxfermanager.h"
#include "message.h"

#include "llagent.h"
#include "llagentwearables.h"
#include "llfloater.h"
#include "llfocusmgr.h"
#include "llinventorybridge.h"
#include "llinventoryfunctions.h"
#include "llinventorymodel.h"
#include "llviewermessage.h"
#include "llviewerwindow.h"
#include "llviewerregion.h"
#include "llappviewer.h"
#include "lldbstrings.h"
#include "llviewerstats.h"
#include "llmutelist.h"
#include "llnotificationsutil.h"
#include "llcallbacklist.h"
#include "llpreview.h"
#include "llviewercontrol.h"
#include "llvoavatarself.h"
#include "llsdutil.h"
#include <deque>

LLInventoryObserver::LLInventoryObserver()
{
}

// virtual
LLInventoryObserver::~LLInventoryObserver()
{
}

void LLInventoryCompletionObserver::changed(U32 mask)
{
	// scan through the incomplete items and move or erase them as
	// appropriate.
	if(!mIncomplete.empty())
	{
		for(item_ref_t::iterator it = mIncomplete.begin(); it < mIncomplete.end(); )
		{
			LLViewerInventoryItem* item = gInventory.getItem(*it);
			if(!item)
			{
				it = mIncomplete.erase(it);
				continue;
			}
			if(item->isComplete())
			{
				mComplete.push_back(*it);
				it = mIncomplete.erase(it);
				continue;
			}
			++it;
		}
		if(mIncomplete.empty())
		{
			done();
		}
	}
}

void LLInventoryCompletionObserver::watchItem(const LLUUID& id)
{
	if(id.notNull())
	{
		mIncomplete.push_back(id);
	}
}


void LLInventoryFetchObserver::changed(U32 mask)
{
	// scan through the incomplete items and move or erase them as
	// appropriate.
	if(!mIncomplete.empty())
	{
		for(item_ref_t::iterator it = mIncomplete.begin(); it < mIncomplete.end(); )
		{
			LLViewerInventoryItem* item = gInventory.getItem(*it);
			if(!item)
			{
				if (mRetryIfMissing)
				{
					// BAP changed to skip these items, so we should keep retrying until they arrive.
					// Did not make this the default behavior because of uncertainty about impact -
					// could cause some observers that currently complete to wait forever.
					++it;
				}
				else
				{
					// BUG: This can cause done() to get called prematurely below.
					// This happens with the LLGestureInventoryFetchObserver that
					// loads gestures at startup. JC
					it = mIncomplete.erase(it);
				}
				continue;
			}
			if(item->isComplete())
			{
				mComplete.push_back(*it);
				it = mIncomplete.erase(it);
				continue;
			}
			++it;
		}
		if(mIncomplete.empty())
		{
			done();
		}
	}
	//llinfos << "LLInventoryFetchObserver::changed() mComplete size " << mComplete.size() << llendl;
	//llinfos << "LLInventoryFetchObserver::changed() mIncomplete size " << mIncomplete.size() << llendl;
}

bool LLInventoryFetchObserver::isEverythingComplete() const
{
	return mIncomplete.empty();
}

void fetch_items_from_llsd(const LLSD& items_llsd)
{
	if (!items_llsd.size()) return;
	LLSD body;
	body[0]["cap_name"] = "FetchInventory";
	body[1]["cap_name"] = "FetchLib";
	for (S32 i=0; i<items_llsd.size();i++)
	{
		if (items_llsd[i]["owner_id"].asString() == gAgent.getID().asString())
		{
			body[0]["items"].append(items_llsd[i]);
			continue;
		}
		if (items_llsd[i]["owner_id"].asString() == ALEXANDRIA_LINDEN_ID.asString())
		{
			body[1]["items"].append(items_llsd[i]);
			continue;
		}
	}
		
	for (S32 i=0; i<body.size(); i++)
	{
		if (0 >= body[i].size()) continue;
		std::string url = gAgent.getRegion()->getCapability(body[i]["cap_name"].asString());

		if (!url.empty())
		{
			body[i]["agent_id"]	= gAgent.getID();
			LLHTTPClient::post(url, body[i], new LLInventoryModel::fetchInventoryResponder(body[i]));
			break;
		}

		LLMessageSystem* msg = gMessageSystem;
		BOOL start_new_message = TRUE;
		for (S32 j=0; j<body[i]["items"].size(); j++)
		{
			LLSD item_entry = body[i]["items"][j];
			if(start_new_message)
			{
				start_new_message = FALSE;
				msg->newMessageFast(_PREHASH_FetchInventory);
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			}
			msg->nextBlockFast(_PREHASH_InventoryData);
			msg->addUUIDFast(_PREHASH_OwnerID, item_entry["owner_id"].asUUID());
			msg->addUUIDFast(_PREHASH_ItemID, item_entry["item_id"].asUUID());
			if(msg->isSendFull(NULL))
			{
				start_new_message = TRUE;
				gAgent.sendReliableMessage();
			}
		}
		if(!start_new_message)
		{
			gAgent.sendReliableMessage();
		}
	}
}

void LLInventoryFetchObserver::fetchItems(
	const LLInventoryFetchObserver::item_ref_t& ids)
{
	LLUUID owner_id;
	LLSD items_llsd;
	for(item_ref_t::const_iterator it = ids.begin(); it < ids.end(); ++it)
	{
		LLViewerInventoryItem* item = gInventory.getItem(*it);
		if(item)
		{
			if(item->isComplete())
			{
				// It's complete, so put it on the complete container.
				mComplete.push_back(*it);
				continue;
			}
			else
			{
				owner_id = item->getPermissions().getOwner();
			}
		}
		else
		{
			// assume it's agent inventory.
			owner_id = gAgent.getID();
		}
		
		// It's incomplete, so put it on the incomplete container, and
		// pack this on the message.
		mIncomplete.push_back(*it);
		
		// Prepare the data to fetch
		LLSD item_entry;
		item_entry["owner_id"] = owner_id;
		item_entry["item_id"] = (*it);
		items_llsd.append(item_entry);
	}
	fetch_items_from_llsd(items_llsd);
}

// virtual
void LLInventoryFetchDescendentsObserver::changed(U32 mask)
{
	for(folder_ref_t::iterator it = mIncompleteFolders.begin(); it < mIncompleteFolders.end();)
	{
		LLViewerInventoryCategory* cat = gInventory.getCategory(*it);
		if(!cat)
		{
			it = mIncompleteFolders.erase(it);
			continue;
		}
		if(isComplete(cat))
		{
			mCompleteFolders.push_back(*it);
			it = mIncompleteFolders.erase(it);
			continue;
		}
		++it;
	}
	if(mIncompleteFolders.empty())
	{
		done();
	}
}

void LLInventoryFetchDescendentsObserver::fetchDescendents(
	const folder_ref_t& ids)
{
	for(folder_ref_t::const_iterator it = ids.begin(); it != ids.end(); ++it)
	{
		LLViewerInventoryCategory* cat = gInventory.getCategory(*it);
		if(!cat) continue;
		if(!isComplete(cat))
		{
			cat->fetchDescendents();		//blindly fetch it without seeing if anything else is fetching it.
			mIncompleteFolders.push_back(*it);	//Add to list of things being downloaded for this observer.
		}
		else
		{
			mCompleteFolders.push_back(*it);
		}
	}
}

bool LLInventoryFetchDescendentsObserver::isEverythingComplete() const
{
	return mIncompleteFolders.empty();
}

bool LLInventoryFetchDescendentsObserver::isComplete(LLViewerInventoryCategory* cat)
{
	const S32 version = cat->getVersion();
	const S32 expected_num_descendents = cat->getDescendentCount();
	if ((version == LLViewerInventoryCategory::VERSION_UNKNOWN) ||
		(expected_num_descendents == LLViewerInventoryCategory::DESCENDENT_COUNT_UNKNOWN))
	{
		return false;
	}
	// it might be complete - check known descendents against
	// currently available.
	LLInventoryModel::cat_array_t* cats;
	LLInventoryModel::item_array_t* items;
	gInventory.getDirectDescendentsOf(cat->getUUID(), cats, items);
	if(!cats || !items)
	{
		llwarns << "Category '" << cat->getName() << "' descendents corrupted, fetch failed." << llendl;
		// NULL means the call failed -- cats/items map doesn't exist (note: this does NOT mean
		// that the cat just doesn't have any items or subfolders).
		// Unrecoverable, so just return done so that this observer can be cleared
		// from memory.
		return true;
	}
	const S32 current_num_known_descendents = cats->count() + items->count();
	
	// Got the number of descendents that we were expecting, so we're done.
	if (current_num_known_descendents == expected_num_descendents)
	{
		return true;
	}

	// Error condition, but recoverable.  This happens if something was added to the
	// category before it was initialized, so accountForUpdate didn't update descendent
	// count and thus the category thinks it has fewer descendents than it actually has.
	if (current_num_known_descendents >= expected_num_descendents)
	{
		llwarns << "Category '" << cat->getName() << "' expected descendentcount:" << expected_num_descendents << " descendents but got descendentcount:" << current_num_known_descendents << llendl;
		cat->setDescendentCount(current_num_known_descendents);
		return true;
	}
	return false;
}

void LLInventoryFetchComboObserver::changed(U32 mask)
{
	if(!mIncompleteItems.empty())
	{
		for(item_ref_t::iterator it = mIncompleteItems.begin(); it < mIncompleteItems.end(); )
		{
			LLViewerInventoryItem* item = gInventory.getItem(*it);
			if(!item)
			{
				it = mIncompleteItems.erase(it);
				continue;
			}
			if(item->isComplete())
		{	
				mCompleteItems.push_back(*it);
				it = mIncompleteItems.erase(it);
				continue;
			}
			++it;
		}
	}
	if(!mIncompleteFolders.empty())
	{
		for(folder_ref_t::iterator it = mIncompleteFolders.begin(); it < mIncompleteFolders.end();)
		{
			LLViewerInventoryCategory* cat = gInventory.getCategory(*it);
			if(!cat)
			{
				it = mIncompleteFolders.erase(it);
				continue;
			}
			if(gInventory.isCategoryComplete(*it))
			{
				mCompleteFolders.push_back(*it);
				it = mIncompleteFolders.erase(it);
				continue;
			}
			++it;
		}
	}
	if(!mDone && mIncompleteItems.empty() && mIncompleteFolders.empty())
	{
		mDone = true;
		done();
	}
}

void LLInventoryFetchComboObserver::fetch(
	const folder_ref_t& folder_ids,
	const item_ref_t& item_ids)
{
	lldebugs << "LLInventoryFetchComboObserver::fetch()" << llendl;
	for(folder_ref_t::const_iterator fit = folder_ids.begin(); fit != folder_ids.end(); ++fit)
	{
		LLViewerInventoryCategory* cat = gInventory.getCategory(*fit);
		if(!cat) continue;
		if(!gInventory.isCategoryComplete(*fit))
		{
			cat->fetchDescendents();
			lldebugs << "fetching folder " << *fit <<llendl;
			mIncompleteFolders.push_back(*fit);
		}
		else
		{
			mCompleteFolders.push_back(*fit);
			lldebugs << "completing folder " << *fit <<llendl;
		}
	}

	// Now for the items - we fetch everything which is not a direct
	// descendent of an incomplete folder because the item will show
	// up in an inventory descendents message soon enough so we do not
	// have to fetch it individually.
	LLSD items_llsd;
	LLUUID owner_id;
	for(item_ref_t::const_iterator iit = item_ids.begin(); iit != item_ids.end(); ++iit)
	{
		LLViewerInventoryItem* item = gInventory.getItem(*iit);
		if(!item)
		{
			lldebugs << "uanble to find item " << *iit << llendl;
			continue;
		}
		if(item->isComplete())
		{
			// It's complete, so put it on the complete container.
			mCompleteItems.push_back(*iit);
			lldebugs << "completing item " << *iit << llendl;
			continue;
		}
		else
		{
			mIncompleteItems.push_back(*iit);
			owner_id = item->getPermissions().getOwner();
		}
		if(std::find(mIncompleteFolders.begin(), mIncompleteFolders.end(), item->getParentUUID()) == mIncompleteFolders.end())
		{
			LLSD item_entry;
			item_entry["owner_id"] = owner_id;
			item_entry["item_id"] = (*iit);
			items_llsd.append(item_entry);
		}
		else
		{
			lldebugs << "not worrying about " << *iit << llendl;
		}
	}
	fetch_items_from_llsd(items_llsd);
}

void LLInventoryExistenceObserver::watchItem(const LLUUID& id)
{
	if(id.notNull())
	{
		mMIA.push_back(id);
	}
}

void LLInventoryExistenceObserver::changed(U32 mask)
{
	// scan through the incomplete items and move or erase them as
	// appropriate.
	if(!mMIA.empty())
	{
		for(item_ref_t::iterator it = mMIA.begin(); it < mMIA.end(); )
		{
			LLViewerInventoryItem* item = gInventory.getItem(*it);
			if(!item)
			{
				++it;
				continue;
			}
			mExist.push_back(*it);
			it = mMIA.erase(it);
		}
		if(mMIA.empty())
		{
			done();
		}
	}
}

void LLInventoryAddedObserver::changed(U32 mask)
{
	if(!(mask & LLInventoryObserver::ADD))
	{
		return;
	}

	// *HACK: If this was in response to a packet off
	// the network, figure out which item was updated.
	LLMessageSystem* msg = gMessageSystem;

	std::string msg_name;
	if (mMessageName.empty())
	{
		msg_name = msg->getMessageName();
	}
	else
	{
		msg_name = mMessageName;
	}

	if (msg_name.empty())
	{
		return;
	}
	
	// We only want newly created inventory items. JC
	if ( msg_name != "UpdateCreateInventoryItem")
	{
		return;
	}

	LLPointer<LLViewerInventoryItem> titem = new LLViewerInventoryItem;
	S32 num_blocks = msg->getNumberOfBlocksFast(_PREHASH_InventoryData);
	for(S32 i = 0; i < num_blocks; ++i)
	{
		titem->unpackMessage(msg, _PREHASH_InventoryData, i);
		if (!(titem->getUUID().isNull()))
		{
			//we don't do anything with null keys
			mAdded.push_back(titem->getUUID());
		}
	}
	if (!mAdded.empty())
	{
		done();
	}
}

LLInventoryTransactionObserver::LLInventoryTransactionObserver(
	const LLTransactionID& transaction_id) :
	mTransactionID(transaction_id)
{
}

void LLInventoryTransactionObserver::changed(U32 mask)
{
	if(mask & LLInventoryObserver::ADD)
	{
		// This could be it - see if we are processing a bulk update
		LLMessageSystem* msg = gMessageSystem;
		if(msg->getMessageName()
		   && (0 == strcmp(msg->getMessageName(), "BulkUpdateInventory")))
		{
			// we have a match for the message - now check the
			// transaction id.
			LLUUID id;
			msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_TransactionID, id);
			if(id == mTransactionID)
			{
				// woo hoo, we found it
				folder_ref_t folders;
				item_ref_t items;
				S32 count;
				count = msg->getNumberOfBlocksFast(_PREHASH_FolderData);
				S32 i;
				for(i = 0; i < count; ++i)
				{
					msg->getUUIDFast(_PREHASH_FolderData, _PREHASH_FolderID, id, i);
					if(id.notNull())
					{
						folders.push_back(id);
					}
				}
				count = msg->getNumberOfBlocksFast(_PREHASH_ItemData);
				for(i = 0; i < count; ++i)
				{
					msg->getUUIDFast(_PREHASH_ItemData, _PREHASH_ItemID, id, i);
					if(id.notNull())
					{
						items.push_back(id);
					}
				}

				// call the derived class the implements this method.
				done(folders, items);
			}
		}
	}
}
