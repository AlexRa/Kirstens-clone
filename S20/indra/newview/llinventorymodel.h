/** 
 * @file llinventorymodel.h
 * @brief LLInventoryModel class header file
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

#ifndef LL_LLINVENTORYMODEL_H
#define LL_LLINVENTORYMODEL_H

#include "llassettype.h"
#include "llfoldertype.h"
#include "lldarray.h"
#include "llframetimer.h"
#include "llhttpclient.h"
#include "lluuid.h"
#include "llpermissionsflags.h"
#include "llstring.h"
#include <map>
#include <set>
#include <string>
#include <vector>

class LLInventoryObserver;
class LLInventoryObject;
class LLInventoryItem;
class LLInventoryCategory;
class LLViewerInventoryItem;
class LLViewerInventoryCategory;
class LLViewerInventoryItem;
class LLViewerInventoryCategory;
class LLMessageSystem;
class LLInventoryCollectFunctor;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLInventoryModel
//
// This class represents a collection of inventory, and provides
// efficient ways to access that information. This class could in
// theory be used for any place where you need inventory, though it
// optimizes for time efficiency - not space efficiency, probably
// making it inappropriate for use on tasks.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


class LLInventoryModel
{
public:
	friend class LLInventoryModelFetchDescendentsResponder;

	enum EHasChildren
	{
		CHILDREN_NO,
		CHILDREN_YES,
		CHILDREN_MAYBE
	};

	// These are used a lot...
	typedef LLDynamicArray<LLPointer<LLViewerInventoryCategory> > cat_array_t;
	typedef LLDynamicArray<LLPointer<LLViewerInventoryItem> > item_array_t;
	typedef std::set<LLUUID> changed_items_t;
	
	// construction & destruction
	LLInventoryModel();
	~LLInventoryModel();
	
	void cleanupInventory();
	
	class fetchInventoryResponder : public LLHTTPClient::Responder
	{
	public:
		fetchInventoryResponder(const LLSD& request_sd) : mRequestSD(request_sd) {};
		void result(const LLSD& content);			
		
		void error(U32 status, const std::string& reason);

	public:
		typedef std::vector<LLViewerInventoryCategory*> folder_ref_t;
	protected:
		LLSD mRequestSD;
	};

	//
	// Accessors
	//

	// Check if one object has a parent chain up to the category specified by UUID.
	BOOL isObjectDescendentOf(const LLUUID& obj_id, const LLUUID& cat_id) const;

	// Get whatever special folder this object is a child of, if any.
	const LLViewerInventoryCategory *getFirstNondefaultParent(const LLUUID& obj_id) const;

	// Get the object by id. Returns NULL if not found.
	// * WARNING: use the pointer returned for read operations - do
	// not modify the object values in place or you will break stuff.
	LLInventoryObject* getObject(const LLUUID& id) const;

	// Get the item by id. Returns NULL if not found.
	// * WARNING: use the pointer for read operations - use the
	// updateItem() method to actually modify values.
	LLViewerInventoryItem* getItem(const LLUUID& id) const;

	// Get the category by id. Returns NULL if not found.
	// * WARNING: use the pointer for read operations - use the
	// updateCategory() method to actually modify values.
	LLViewerInventoryCategory* getCategory(const LLUUID& id) const;

	// Return the number of items or categories
	S32 getItemCount() const;
	S32 getCategoryCount() const;

	// Return the direct descendents of the id provided.Set passed
	// in values to NULL if the call fails.
	// *WARNING: The array provided points straight into the guts of
	// this object, and should only be used for read operations, since
	// modifications may invalidate the internal state of the
	// inventory.
	void getDirectDescendentsOf(const LLUUID& cat_id,
								cat_array_t*& categories,
								item_array_t*& items) const;
	
	// SJB: Added version to lock the arrays to catch potential logic bugs
	void lockDirectDescendentArrays(const LLUUID& cat_id,
									cat_array_t*& categories,
									item_array_t*& items);
	void unlockDirectDescendentArrays(const LLUUID& cat_id);
	
	// Starting with the object specified, add its descendents to the
	// array provided, but do not add the inventory object specified
	// by id. There is no guaranteed order. Neither array will be
	// erased before adding objects to it. Do not store a copy of the
	// pointers collected - use them, and collect them again later if
	// you need to reference the same objects.
	enum { EXCLUDE_TRASH = FALSE, INCLUDE_TRASH = TRUE };
	void collectDescendents(const LLUUID& id,
							cat_array_t& categories,
							item_array_t& items,
							BOOL include_trash);
	void collectDescendentsIf(const LLUUID& id,
							  cat_array_t& categories,
							  item_array_t& items,
							  BOOL include_trash,
							  LLInventoryCollectFunctor& add,
							  BOOL follow_folder_links = FALSE);

	// Collect all items in inventory that are linked to item_id.
	// Assumes item_id is itself not a linked item.
	item_array_t collectLinkedItems(const LLUUID& item_id,
									const LLUUID& start_folder_id = LLUUID::null);

	// Get the inventoryID that this item points to, else just return item_id
	const LLUUID& getLinkedItemID(const LLUUID& object_id) const;

	// The inventory model usage is sensitive to the initial construction of the 
	// model. 
	bool isInventoryUsable() const;

	//
	// Mutators
	//

	// Calling this method with an inventory item will either change
	// an existing item with a matching item_id, or will add the item
	// to the current inventory. Returns the change mask generated by
	// the update. No notification will be sent to observers. This
	// method will only generate network traffic if the item had to be
	// reparented.
	// *NOTE: In usage, you will want to perform cache accounting
	// operations in LLInventoryModel::accountForUpdate() or
	// LLViewerInventoryItem::updateServer() before calling this
	// method.
	U32 updateItem(const LLViewerInventoryItem* item);

	// Calling this method with an inventory category will either
	// change an existing item with the matching id, or it will add
	// the category. No notifcation will be sent to observers. This
	// method will only generate network traffic if the item had to be
	// reparented.
	// *NOTE: In usage, you will want to perform cache accounting
	// operations in LLInventoryModel::accountForUpdate() or
	// LLViewerInventoryCategory::updateServer() before calling this
	// method.
	void updateCategory(const LLViewerInventoryCategory* cat);

	// This method will move the specified object id to the specified
	// category, update the internal structures. No cache accounting,
	// observer notification, or server update is performed.
	void moveObject(const LLUUID& object_id, const LLUUID& cat_id);

	// delete a particular inventory object by ID. This will purge one
	// object from the internal data structures maintaining a
	// consistent internal state. No cache accounting, observer
	// notification, or server update is performed.  Purges linked items.
	void deleteObject(const LLUUID& id);
	
	// delete a particular inventory object by ID, and delete it from
	// the server.  Also updates linked items.
	void purgeObject(const LLUUID& id);
	void updateLinkedObjectsFromPurge(const LLUUID& baseobj_id);

	// This is a method which collects the descendants of the id
	// provided. If the category is not found, no action is
	// taken. This method goes through the long winded process of
	// removing server representation of folders and items while doing
	// cache accounting in a fairly efficient manner. This method does
	// not notify observers (though maybe it should...)
	void purgeDescendentsOf(const LLUUID& id);

	// This method optimally removes the referenced categories and
	// items from the current agent's inventory in the database. It
	// performs all of the during deletion. The local representation
	// is not removed.
	void deleteFromServer(LLDynamicArray<LLUUID>& category_ids,
						  LLDynamicArray<LLUUID>& item_ids);

	// Add/remove an observer. If the observer is destroyed, be sure
	// to remove it.
	void addObserver(LLInventoryObserver* observer);
	void removeObserver(LLInventoryObserver* observer);
	BOOL containsObserver(LLInventoryObserver* observer) const;

	//
	// Misc Methods 
	//

	// findCategoryUUIDForType() returns the uuid of the category that
	// specifies 'type' as what it defaults to containing. The
	// category is not necessarily only for that type. *NOTE: If create_folder is true, this
	// will create a new inventory category on the fly if one does not exist. *NOTE: if find_in_library is
	// true it will search in the user's library folder instead of "My Inventory"
	// SDK: Added flag to specify whether the folder should be created if not found.  This fixes the horrible
	// multiple trash can bug.
	const LLUUID findCategoryUUIDForType(LLFolderType::EType preferred_type, bool create_folder = true, bool find_in_library = false);

	// This gets called by the idle loop.  It only updates if new
	// state is detected.  Call notifyObservers() manually to update
	// regardless of whether state change has been indicated.
	void idleNotifyObservers();

	// Call this method to explicitly update everyone on a new state.
	// The optional argument 'service_name' is used by Agent Inventory Service [DEV-20328]
	void notifyObservers(const std::string service_name="");

	// This allows outsiders to tell the inventory if something has
	// been changed 'under the hood', but outside the control of the
	// inventory. For example, if we grant someone modify permissions,
	// then that changes the data structures for LLAvatarTracker, but
	// potentially affects inventory observers. This API makes sure
	// that the next notify will include that notification.
	void addChangedMask(U32 mask, const LLUUID& referent);

	const changed_items_t& getChangedIDs() const { return mChangedItemIDs; }

	// This method to prepares a set of mock inventory which provides
	// minimal functionality before the actual arrival of inventory.
	//void mock(const LLUUID& root_id);

	// Make sure we have the descendents in the structure.  Returns true
	// if a fetch was performed.
	bool fetchDescendentsOf(const LLUUID& folder_id);

	// call this method to request the inventory.
	//void requestFromServer(const LLUUID& agent_id);

	// call this method on logout to save a terse representation
	void cache(const LLUUID& parent_folder_id, const LLUUID& agent_id);

	// Generates a string containing the path to the item specified by
	// item_id.
	void appendPath(const LLUUID& id, std::string& path) const;

	// message handling functionality
	static void registerCallbacks(LLMessageSystem* msg);

	// Convenience function to create a new category. You could call
	// updateCatgory() with a newly generated UUID category, but this
	// version will take care of details like what the name should be
	// based on preferred type. Returns the UUID of the new
	// category. If you want to use the default name based on type,
	// pass in a NULL to the 'name parameter.
	LLUUID createNewCategory(const LLUUID& parent_id,
							 LLFolderType::EType preferred_type,
							 const std::string& name);

	// methods to load up inventory skeleton & meat. These are used
	// during authentication. return true if everything parsed.
	bool loadSkeleton(const LLSD& options, const LLUUID& owner_id);
	bool loadMeat(const LLSD& options, const LLUUID& owner_id);

	// This is a brute force method to rebuild the entire parent-child
	// relations.
	void buildParentChildMap();

	//
	// Category accounting.
	//

	// This structure represents the number of items added or removed
	// from a category.
	struct LLCategoryUpdate
	{
		LLCategoryUpdate() : mDescendentDelta(0) {}
		LLCategoryUpdate(const LLUUID& category_id, S32 delta) :
			mCategoryID(category_id),
			mDescendentDelta(delta) {}
		LLUUID mCategoryID;
		S32 mDescendentDelta;
	};
	typedef std::vector<LLCategoryUpdate> update_list_t;

	// This structure eixts to make it easier to account for deltas in
	// a map.
	struct LLInitializedS32
	{
		LLInitializedS32() : mValue(0) {}
		LLInitializedS32(S32 value) : mValue(value) {}
		S32 mValue;
		LLInitializedS32& operator++() { ++mValue; return *this; }
		LLInitializedS32& operator--() { --mValue; return *this; }
	};
	typedef std::map<LLUUID, LLInitializedS32> update_map_t;

	// Call these methods when there are category updates, but call
	// them *before* the actual update so the method can do descendent
	// accounting correctly.
	void accountForUpdate(const LLCategoryUpdate& update) const;
	void accountForUpdate(const update_list_t& updates);
	void accountForUpdate(const update_map_t& updates);

	// Return child status of category children. yes/no/maybe
	EHasChildren categoryHasChildren(const LLUUID& cat_id) const;

	// returns true iff category version is known and theoretical
	// descendents == actual descendents.
	bool isCategoryComplete(const LLUUID& cat_id) const;
	
	// callbacks
	// Trigger a notification and empty the folder type (FT_TRASH or FT_LOST_AND_FOUND) if confirmed
	void emptyFolderType(const std::string notification, LLFolderType::EType folder_type);
	bool callbackEmptyFolderType(const LLSD& notification, const LLSD& response, LLFolderType::EType preferred_type);

	// Utility Functions
	void removeItem(const LLUUID& item_id);
	
    static void findLostItems();

	// Data about the agent's root folder and root library folder
	// are stored here, rather than in LLAgent where it used to be, because
	// gInventory is a singleton and represents the agent's inventory.
	// The "library" is actually the inventory of a special agent,
	// usually Alexandria Linden.
	const LLUUID &getRootFolderID() const;
	const LLUUID &getLibraryOwnerID() const;
	const LLUUID &getLibraryRootFolderID() const;

	// These are set during login with data from the server
	void setRootFolderID(const LLUUID& id);
	void setLibraryOwnerID(const LLUUID& id);
	void setLibraryRootFolderID(const LLUUID& id);


	/**
	 * Changes items order by insertion of the item identified by src_item_id
	 * BEFORE the item identified by dest_item_id. Both items must exist in items array.
	 *
	 * Sorting is stored after method is finished. Only src_item_id is moved before dest_item_id.
	 *
	 * @param[in, out] items - vector with items to be updated. It should be sorted in a right way
	 * before calling this method.
	 * @param src_item_id - LLUUID of inventory item to be moved in new position
	 * @param dest_item_id - LLUUID of inventory item before which source item should be placed.
	 */
	static void updateItemsOrder(LLInventoryModel::item_array_t& items, const LLUUID& src_item_id, const LLUUID& dest_item_id);

	/**
	 * Saves current order of the passed items using inventory item sort field.
	 *
	 * It reset items' sort fields and saves them on server.
	 * Is used to save order for Favorites folder.
	 *
	 * @param[in] items vector of items in order to be saved.
	 */
	void saveItemsOrder(const LLInventoryModel::item_array_t& items);

	/**
	 * Rearranges Landmarks inside Favorites folder.
	 * Moves source landmark before target one.
	 *
	 * @param source_item_id - LLUUID of the source item to be moved into new position
	 * @param target_item_id - LLUUID of the target item before which source item should be placed.
	 */
	void rearrangeFavoriteLandmarks(const LLUUID& source_item_id, const LLUUID& target_item_id);

protected:

	// Internal methods which add inventory and make sure that all of
	// the internal data structures are consistent. These methods
	// should be passed pointers of newly created objects, and the
	// instance will take over the memory management from there.
	void addCategory(LLViewerInventoryCategory* category);
	void addItem(LLViewerInventoryItem* item);

	// ! DEPRECRATE ! Remove this and add it into findCategoryUUIDForType,
	// since that's the only function that uses this.  It's too confusing 
	// having both methods.
	// 
	// Internal method which looks for a category with the specified
	// preferred type. Returns LLUUID::null if not found
 	const LLUUID &findCatUUID(LLFolderType::EType preferred_type, bool find_in_library = false) const;

	// Empty the entire contents
	void empty();

	// Given the current state of the inventory items, figure out the
	// clone information. *FIX: This is sub-optimal, since we can
	// insert this information snurgically, but this makes sure the
	// implementation works before we worry about optimization.
	//void recalculateCloneInformation();

	// file import/export.
	static bool loadFromFile(const std::string& filename,
							 cat_array_t& categories,
							 item_array_t& items,
							 bool& is_cache_obsolete); 
	static bool saveToFile(const std::string& filename,
						   const cat_array_t& categories,
						   const item_array_t& items); 

	// message handling functionality
	//static void processUseCachedInventory(LLMessageSystem* msg, void**);
	static void processUpdateCreateInventoryItem(LLMessageSystem* msg, void**);
	static void processRemoveInventoryItem(LLMessageSystem* msg, void**);
	static void processUpdateInventoryFolder(LLMessageSystem* msg, void**);
	static void processRemoveInventoryFolder(LLMessageSystem* msg, void**);
	//static void processExchangeCallingcard(LLMessageSystem* msg, void**);
	//static void processAddCallingcard(LLMessageSystem* msg, void**);
	//static void processDeclineCallingcard(LLMessageSystem* msg, void**);
	static void processSaveAssetIntoInventory(LLMessageSystem* msg, void**);
	static void processBulkUpdateInventory(LLMessageSystem* msg, void**);
	static void processInventoryDescendents(LLMessageSystem* msg, void**);
	static void processMoveInventoryItem(LLMessageSystem* msg, void**);
	static void processFetchInventoryReply(LLMessageSystem* msg, void**);
	
	bool messageUpdateCore(LLMessageSystem* msg, bool do_accounting);

	// Updates all linked items pointing to this id.
	void addChangedMaskForLinks(const LLUUID& object_id, U32 mask);

protected:
	cat_array_t* getUnlockedCatArray(const LLUUID& id);
	item_array_t* getUnlockedItemArray(const LLUUID& id);
	
private:
	// Variables used to track what has changed since the last notify.
	U32 mModifyMask;
	changed_items_t mChangedItemIDs;

	std::map<LLUUID, bool> mCategoryLock;
	std::map<LLUUID, bool> mItemLock;
	
	// cache recent lookups
	mutable LLPointer<LLViewerInventoryItem> mLastItem;

	// This last set of indices is used to map parents to children.
	typedef std::map<LLUUID, cat_array_t*> parent_cat_map_t;
	typedef std::map<LLUUID, item_array_t*> parent_item_map_t;
	parent_cat_map_t mParentChildCategoryTree;
	parent_item_map_t mParentChildItemTree;

	typedef std::set<LLInventoryObserver*> observer_list_t;
	observer_list_t mObservers;

	// Agent inventory folder information.
	LLUUID mRootFolderID;
	LLUUID mLibraryRootFolderID;
	LLUUID mLibraryOwnerID;

	static BOOL sTimelyFetchPending;
	static S32  sNumFetchRetries;
	static LLFrameTimer sFetchTimer;
	static F32 sMinTimeBetweenFetches;
	static F32 sMaxTimeBetweenFetches;

	// Expected inventory cache version
	const static S32 sCurrentInvCacheVersion;
	
	// This flag is used to handle an invalid inventory state.
	bool mIsAgentInvUsable;

private:
	// Information for tracking the actual inventory. We index this
	// information in a lot of different ways so we can access
	// the inventory using several different identifiers.
	// mInventory member data is the 'master' list of inventory, and
	// mCategoryMap and mItemMap store uuid->object mappings. 
	typedef std::map<LLUUID, LLPointer<LLViewerInventoryCategory> > cat_map_t;
	typedef std::map<LLUUID, LLPointer<LLViewerInventoryItem> > item_map_t;
	//inv_map_t mInventory;
	cat_map_t mCategoryMap;
	item_map_t mItemMap;

	// Flag set when notifyObservers is being called, to look for bugs
	// where it's called recursively.
	BOOL mIsNotifyObservers;
public:
	// *NOTE: DEBUG functionality
	void dumpInventory() const;


	////////////////////////////////////////////////////////////////////////////////
	// Bulk fetch
public:
	// Start and stop background breadth-first fetching of inventory contents.
	// This gets triggered when performing a filter-search
	void startBackgroundFetch(const LLUUID& cat_id = LLUUID::null);
	static BOOL backgroundFetchActive();
	static bool isEverythingFetched();
	static void backgroundFetch(void*); // background fetch idle function
	static void incrBulkFetch(S16 fetching) {  sBulkFetchCount+=fetching; if (sBulkFetchCount<0) sBulkFetchCount=0; }
	static void stopBackgroundFetch(); // stop fetch process
	static bool isBulkFetchProcessingComplete();

	// Add categories to a list to be fetched in bulk.
	static void bulkFetch(std::string url);

	static bool libraryFetchStarted();
	static bool libraryFetchCompleted();
	static bool libraryFetchInProgress();
	
	static bool myInventoryFetchStarted();
	static bool myInventoryFetchCompleted();
	static bool myInventoryFetchInProgress();
	
private:
 	static BOOL sMyInventoryFetchStarted;
	static BOOL sLibraryFetchStarted;
	static BOOL sAllFoldersFetched; 
	static void setAllFoldersFetched();

	// completing the fetch once per session should be sufficient
	static BOOL sBackgroundFetchActive;
	static S16 sBulkFetchCount;

	////////////////////////////////////////////////////////////////////////////////
	// Login status
public:
	static BOOL getIsFirstTimeInViewer2();
private:
	static BOOL sFirstTimeInViewer2;
};

// a special inventory model for the agent
extern LLInventoryModel gInventory;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLInventoryCollectFunctor
//
// Base class for LLInventoryModel::collectDescendentsIf() method
// which accepts an instance of one of these objects to use as the
// function to determine if it should be added. Derive from this class
// and override the () operator to return TRUE if you want to collect
// the category or item passed in.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLInventoryCollectFunctor
{
public:
	virtual ~LLInventoryCollectFunctor(){};
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item) = 0;

	static bool itemTransferCommonlyAllowed(LLInventoryItem* item);
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLAssetIDMatches
//
// This functor finds inventory items pointing to the specified asset
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLViewerInventoryItem;

class LLAssetIDMatches : public LLInventoryCollectFunctor
{
public:
	LLAssetIDMatches(const LLUUID& asset_id) : mAssetID(asset_id) {}
	virtual ~LLAssetIDMatches() {}
	bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
	
protected:
	LLUUID mAssetID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLLinkedItemIDMatches
//
// This functor finds inventory items linked to the specific inventory id.
// Assumes the inventory id is itself not a linked item.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLLinkedItemIDMatches : public LLInventoryCollectFunctor
{
public:
	LLLinkedItemIDMatches(const LLUUID& item_id) : mBaseItemID(item_id) {}
	virtual ~LLLinkedItemIDMatches() {}
	bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
	
protected:
	LLUUID mBaseItemID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLIsType
//
// Implementation of a LLInventoryCollectFunctor which returns TRUE if
// the type is the type passed in during construction.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLIsType : public LLInventoryCollectFunctor
{
public:
	LLIsType(LLAssetType::EType type) : mType(type) {}
	virtual ~LLIsType() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLIsNotType
//
// Implementation of a LLInventoryCollectFunctor which returns FALSE if the
// type is the type passed in during construction, otherwise false.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLIsNotType : public LLInventoryCollectFunctor
{
public:
	LLIsNotType(LLAssetType::EType type) : mType(type) {}
	virtual ~LLIsNotType() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
};

class LLIsTypeWithPermissions : public LLInventoryCollectFunctor
{
public:
	LLIsTypeWithPermissions(LLAssetType::EType type, const PermissionBit perms, const LLUUID &agent_id, const LLUUID &group_id) 
		: mType(type), mPerm(perms), mAgentID(agent_id), mGroupID(group_id) {}
	virtual ~LLIsTypeWithPermissions() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
	PermissionBit mPerm;
	LLUUID			mAgentID;
	LLUUID			mGroupID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLIsClone
//
// Implementation of a LLInventoryCollectFunctor which returns TRUE if
// the object is a clone of the item passed in during
// construction.
//
// *NOTE: Since clone information is determined based off of asset id
// (or creator with calling cards), if the id is NULL, it has no
// clones - even itself.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//class LLIsClone : public LLInventoryCollectFunctor
//{
//public:
//	LLIsClone(LLViewerInventoryItem* item) : mItem(item) {}
//	virtual ~LLIsClone() {}
//	virtual bool operator()(LLViewerInventoryCategory* cat,
//							LLViewerInventoryItem* item);
//protected:
//	LLPointer<LLViewerInventoryItem> mItem;
//};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLBuddyCollector
//
// Simple class that collects calling cards that are not null, and not
// the agent. Duplicates are possible.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLBuddyCollector : public LLInventoryCollectFunctor
{
public:
	LLBuddyCollector() {}
	virtual ~LLBuddyCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLUniqueBuddyCollector
//
// Simple class that collects calling cards that are not null, and not
// the agent. Duplicates are discarded.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLUniqueBuddyCollector : public LLInventoryCollectFunctor
{
public:
	LLUniqueBuddyCollector() {}
	virtual ~LLUniqueBuddyCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);

protected:
	std::set<LLUUID> mSeen;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLParticularBuddyCollector
//
// Simple class that collects calling cards that match a particular uuid
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLParticularBuddyCollector : public LLInventoryCollectFunctor
{
public:
	LLParticularBuddyCollector(const LLUUID& id) : mBuddyID(id) {}
	virtual ~LLParticularBuddyCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLUUID mBuddyID;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLNameCategoryCollector
//
// Collects categories based on case-insensitive match of prefix
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLNameCategoryCollector : public LLInventoryCollectFunctor
{
public:
	LLNameCategoryCollector(const std::string& name) : mName(name) {}
	virtual ~LLNameCategoryCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	std::string mName;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindCOFValidItems
//
// Collects items that can be legitimately linked to in the COF.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindCOFValidItems : public LLInventoryCollectFunctor
{
public:
	LLFindCOFValidItems() {}
	virtual ~LLFindCOFValidItems() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
	
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindWearables
//
// Collects wearables based on item type.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindWearables : public LLInventoryCollectFunctor
{
public:
	LLFindWearables() {}
	virtual ~LLFindWearables() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
};

#endif // LL_LLINVENTORYMODEL_H

