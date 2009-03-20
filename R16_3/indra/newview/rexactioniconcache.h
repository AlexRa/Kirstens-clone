/** 
 * @file REXActionIconCache.h
 * @brief REXActionIconCache class header file
 Copyright (c) 2007-2008 adminotech
 For conditions of distribution and use, see copyright notice in license.txt
 */

#ifndef REX_REXACTIONICONCACHE_H
#define REX_REXACTIONICONCACHE_H


#include "llhttpclient.h"
#include "llcontrol.h"
#include "sha1.h"
#include <RexAvatar.h>

class LLPanelActionIcons;

class REXActionIconResponder : public LLHTTPClient::Responder
{
public:
   //! Constructor that specifies actionicons
   
    //   \param actionicons realXtend actionicons
   
   explicit REXActionIconResponder(LLPanelActionIcons *actionicons, bool addRequest = true);

   //! destructor
   virtual ~REXActionIconResponder();
   
   //! Error occurred, handle it
   virtual void error(U32 status, const std::string& reason);	// called with bad status codes

   //! Parse the received content and handle it
   virtual void result(const LLSD& content);

   //! Cancel this request, do not respond to it.
   virtual void cancel(void);

protected:
   
   //! Map of outstanding requests
   static std::map<LLPanelActionIcons*, REXActionIconResponder*> smPendingRequests;

   //! Avatar object that uses this responder
   LLPanelActionIcons *mIcons;

   //! Cancel current request
   bool mCancelRequest;
};

/*! Cache for realXtend actionicons assets.
    Caches all assets and actionicons data for the user's actionicons.
    Avatar data for other actioniconss not currently cached.
*/
class REXActionIconCache
{
private:
   //! entry in cache for asset
   struct AssetEntry
   {
      std::string mName;
      LLUUID mUid;
   };

public:
   //! Returns singleton instance
   static REXActionIconCache &getSingleton();

   //! Returns singleton instance pointer
   static REXActionIconCache *getSingletonPtr();

   //! Requests update from avatar storage for specified actionicons
   /*!
       \param actionicons realXtend Avatar
   */
   void requestUpdate(LLPanelActionIcons *actionicons);

   //! Sends a request for asset to avatar storage
   /*!
       \param avatar realXtend avatar
       \param hash Asset hash in base64
       \param type Asset type
   */
  // void requestAssetBase64(LLPanelActionIcons *actionicons, const std::string &name, const std::string &hash, LLAssetType::EType type);


   //! Returns true if cache contains asset
   /*!
       \param hash Asset hash in base64
   */
   bool hasAssetBase64(const std::string &hash) const;

   //! Returns true if the asset exists non-cached locally (in rex lib)
   /*!
       \param name name of the asset
       \param type asset type
   */
//   bool hasLocalNonCachedAsset(const std::string &name, LLAssetType::EType type);

   //! Creates new actionicons asset of type type from data
   /*!
       \param data Asset asset
       \param hash Asset hash in base64 encoded form
       \param type Asset type
   */
 //  bool createAsset(const std::string &name, const LLSD::Binary &asset, const std::string &hash, LLAssetType::EType type);

   //! Sets uri for avatar storage. Must be set before requesting assets from storage.
 //  void setStorageUri(const LLString &uri);

   //! Gets uri for avatar storage.
//   LLString getStorageUri() const;

   //! Returns asset with hash from cache
   /*!
       \param hash Asset hash
       \param type Asset type
       \return asset
   */
 //  CacheActionIconAsset &getAsset(const std::string &hash, LLAssetType::EType type);

   //! Saves all current assets to disk.
 //  void saveAssets() const;

   //! Helper function for validating asset type. Avatar cache only supports certain asset types
   /*!
       \param type Asset type
       \return True if asset type is valid for this cache, false otherwise
   */
 //  bool validateType(LLAssetType::EType type);

   //! Constructs equivalent actionicons cache index filename from asset cache index filename 
   static std::string constructAssetCacheFile(const std::string &assetIndexFile);

   //! Calculates SHA1 hash for binary data and encodes it to base64
   static std::string calculateHash(U8 *data, size_t dataSize);

protected:
   //! default constructor
   REXActionIconCache();

   //! destructor
   ~REXActionIconCache();

   //! Load cache data from file
   bool loadCacheData();

   //! Parse cache entries from cache data file. The file must be loaded first before calling this.
   bool parseCacheEntries();

   //! Creates a new cache data file, overwrites old one so be careful!
   void createCacheDataFile();

   //! Returns true if the asset exists non-cached locally (in rex lib). Does not do Ogre/SL context switching, so should not be called directly.
   /*!
       \param name name of the asset
       \param type asset type
   */
 //  bool _hasLocalNonCachedAsset(const std::string &name, LLAssetType::EType type);

   //! Helper function for getting a string from llsd binary. Assumes that the data can be converted directly to string.
//   std::string binToString(const LLSD::Binary data) const;

   //! Filename for actionicons cache data file
   static const char *DEFAULT_ACTIONICONCACHE_FILE;

   //! Uri to assert storage
   LLString mStorageUri;

   //! Contains metadata about cached assets. Unfortunately we need to keep track of cached assets.
   Rex::XmlDocument mCacheData;

   //! path to cache data file
   LLString mCacheDataFile;

   //! Map of all entries in the cache
   std::map<std::string, AssetEntry> mCacheEntries;

   //! Map of assets which are loaded into memory
 //  std::map<std::string, CacheActionIconAsset> mLoadedAssets;

   //! List of assets being retrieved from storage
 //  std::map<std::string, LLHTTPClient::ResponderPtr> mRetrievedAssets;
};

#endif // LL_LLAVATARCACHE_H
