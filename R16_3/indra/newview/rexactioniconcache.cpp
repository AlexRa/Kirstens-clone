/** 
 * @file REXActionIconCache.cpp
 * @brief REXActionIconCache class implementation
 *
 Copyright (c) 2007-2008 adminotech
 For conditions of distribution and use, see copyright notice in license.txt
 */

#include "llviewerprecompiledheaders.h"
#include "rexactioniconcache.h"
#include "llpanelactionicons.h"
#include "llavatar.h"
#include "tinyxml.h"
#include "llbase64.h"
#include "lluuidaa.h"
#include "llsdserialize.h"



const char *REXActionIconCache::DEFAULT_ACTIONICONCACHE_FILE = "actionicons_assets.xml";
/*
CacheActionIconAsset::CacheActionIconAsset() : mLoaded(false)
{
}

CacheActionIconAsset::~CacheActionIconAsset()
{
}

void CacheActionIconAsset::load(const std::string name, const LLUUID &id, LLAssetType::EType type)
{  

   if (gVFS->getExists(id, type))
   {
      LLVFile *file = new LLVFile(gVFS, id, type, LLVFile::READ);
      if (file->getSize() > 0)
      {
         mData.resize(file->getSize());
         file->read(&mData[0], file->getSize(), false);

         mLoaded = true;
         mName = name;
      }
      delete file;
   } else
   {
      llwarns << "File does not exist for asset: " << name << " uuid:" << id.asString() << llendl;
   }
}


bool CacheActionIconAsset::isLoaded() const
{
   return mLoaded;
}

//! Returns asset data
const LLSD::Binary &CacheActionIconAsset::getData() const
{
   return mData;
}

void CacheActionIconAsset::setOgreResourceName(const std::string &name)
{
   mOgreResourceName = name;
}

const std::string &CacheActionIconAsset::getOgreResourceName() const
{
   return mOgreResourceName;
}

const std::string &CacheActionIconAsset::getName() const
{
   return mName;
}

LLAssetType::EType CacheActionIconAsset::getAssetTypeFromExtension(const std::string &name)
{
   size_t foundPos = name.find_last_of(".");
   if (foundPos != std::string::npos && ++foundPos < name.size())
   {
      std::string extension = name.substr(foundPos, name.size() - foundPos);
      if (extension.compare("mesh") == 0)
         return LLAssetType::AT_MESH;

      if (extension.compare("skeleton") == 0)
         return LLAssetType::AT_SKELETON;

      if (extension.compare("material") == 0)
         return LLAssetType::AT_MATERIAL;

      if (extension.compare("png") == 0)
         return LLAssetType::AT_TEXTURE;

      if (extension.compare("dds") == 0)
         return LLAssetType::AT_TEXTURE;

      if (extension.compare("tga") == 0)
         return LLAssetType::AT_TEXTURE;

      if (extension.compare("bmp") == 0)
         return LLAssetType::AT_TEXTURE;

      if (extension.compare("jpg") == 0)
         return LLAssetType::AT_TEXTURE;

      if (extension.compare("jpeg") == 0)
         return LLAssetType::AT_TEXTURE;

      if (extension.compare("cloth") == 0)
         return LLAssetType::AT_PHYSICS_CLOTH;
   }
   return LLAssetType::AT_NONE;
}

*/
std::map<LLPanelActionIcons*, REXActionIconResponder*> REXActionIconResponder::smPendingRequests;

REXActionIconResponder::REXActionIconResponder(LLPanelActionIcons *actionicons, bool addRequest) : LLHTTPClient::Responder(), mIcons(actionicons), mCancelRequest(false)
{
   if (addRequest)
   {
      // See if there already is a pending request for this actionicons
      if (smPendingRequests.find(actionicons) != smPendingRequests.end())
      {
         smPendingRequests[actionicons]->cancel();

         // Abort all asset requests
//         LLActionIconAssetResponder::abortRequests(actionicons);
      }

      smPendingRequests[actionicons] = this;
   }
}


REXActionIconResponder::~REXActionIconResponder()
{
   smPendingRequests.erase(mIcons);
}

void REXActionIconResponder::cancel(void)
{
   mCancelRequest = true;
}
   
void REXActionIconResponder::error(U32 status, const std::string& reason)
{
   if (mCancelRequest)
      return;

  // if (mIcons->getIsSelf() == false)
    //  return;

   llwarns << "Failed to connect to avatar storage: " << reason << llendl;
//   llinfos << "Fetching local avatar..." << llendl;

 //  if (mIcons->importLocal() == true)
 //     return;

   std::string account = gSavedSettings.getString("UserName");
   if (account.empty())
      return;



   llinfos << "Couldn't find local copy of actionicons, fetching actionicons from cache..." << llendl;
   // Get cached version
   std::string hash = REXActionIconCache::calculateHash(reinterpret_cast<U8*>(const_cast<char*>(account.c_str())), account.size());
   
   if (REXActionIconCache::getSingleton().hasAssetBase64(hash) == true)
   {
 /*     CacheActionIconAsset &createdAsset = REXActionIconCache::getSingleton().getAsset(hash, LLAssetType::AT_GENERIC_AVATAR);

      LLSD content;
      std::string str;
      str.resize(createdAsset.getData().size());
      memcpy(&str[0], &(createdAsset.getData()[0]), createdAsset.getData().size());
      std::istringstream istr(str);
      LLSDSerialize::fromXML(content, istr);
      result(content);*/
   }

   //! \todo to error handling / reporting (popup to user about not being able to reach avatar storage server?)
}

void REXActionIconResponder::result(const LLSD& content)
{
   if (!mIcons)
      return;

   if (mCancelRequest)
      return;

   llinfos << "Received reply from avatar storage server." << llendl;
/*
   LLSD::String uid;
   LLSD::String base_name;
   LLSD::String xml_data;

   LLSD::String mesh;
   LLSD::String skeleton;
   LLSD::String texture;

   //! \todo check for proper data
   if (content.has("generic xml") == false)
   {
      llwarns << "No generic xml field!" << llendl;
      return;
   }


   LLSD::map_const_iterator &iter = content.beginMap();
   for ( ; iter != content.endMap() ; ++iter)
   {
      if (iter->first.compare("generic xml") != 0 && iter->first.compare("name") != 0)
      {
         mIcons->assetLoading();
      }
   }
   

   iter = content.beginMap();
   std::string xmlData;
   for ( ; iter != content.endMap() ; ++iter)
   {
      if (iter->first.compare("generic xml") == 0)
      {
         LLSD element = iter->second;

         mIcons->setImportDocument(element.asString());
      }
      else if (iter->first.compare("name") == 0)
      {
      //   llinfos << "Avatar name:" << llendl;
      } else
      {
         LLSD element = iter->second;

         // get asset type from extension for now
         std::string name = iter->first;
         LLAssetType::EType type = LLAssetType::AT_TEXTURE;
         type = CacheActionIconAsset::getAssetTypeFromExtension(name);
         if (type == LLAssetType::AT_NONE)
         {
         //   iter++;
            llwarns << "Asset " << name << " has unknown type, ignoring it." << llendl;

            continue;
         }
         llinfos << "Asset received: " << name << llendl;
         
         if (REXActionIconCache::getSingleton().hasLocalNonCachedAsset(iter->first, type) == true)
         {
            llinfos << "Fetching asset " << iter->first << " from local source." << llendl;
            mIcons->assetReady(iter->first, type);
         } else
         {
            if (REXActionIconCache::getSingleton().hasAssetBase64(element.asString()) == false)
            {
               llinfos << "Requesting asset " << iter->first << " from avatar storage server." << llendl;
               // no copy of the asset locally, get it from avatar storage
               REXActionIconCache::getSingleton().requestAssetBase64(mIcons, iter->first, element.asString(), type);
            } else
            {
               // got a copy of the asset in the cache
               CacheActionIconAsset &createdAsset = REXActionIconCache::getSingleton().getAsset(element.asString(), type);
               if (createdAsset.isLoaded())
               {
                  llinfos << "Fetching asset " << name << " from local cache." << llendl;
                  Ogre::String resourceName = mIcons->createOgreAsset(iter->first, createdAsset, type);
                  createdAsset.setOgreResourceName(resourceName);
                  mIcons->assetReady(resourceName, type);
               } else
               {
                  llwarns << "Error occurred while trying to load asset " << name << " from cache. Trying to fetch asset from asset storage." << llendl;
                  REXActionIconCache::getSingleton().requestAssetBase64(mIcons, iter->first, element.asString(), type);
               }
            }
         }
      }

     // iter++;
   }

   if (mIcons->getIsSelf())
   {
      std::ostringstream stream;
      LLSDSerialize::toXML(content, stream);

      llinfos << "Storing actionicons data to cache." << llendl;
      LLSD::Binary asset;
         
      asset.resize(stream.str().size());
      memcpy(&asset[0], stream.str().c_str(), stream.str().size());


      std::string account = gSavedSettings.getString("UserName");
      if (account.empty())
         return;
      std::string hash = REXActionIconCache::calculateHash(reinterpret_cast<U8*>(const_cast<char*>(account.c_str())), account.size());
      
      if (REXActionIconCache::getSingleton().hasAssetBase64(hash) == false)
         REXActionIconCache::getSingleton().createAsset("actionicons_data", asset, hash, LLAssetType::AT_GENERIC_AVATAR);
   }*/
}



/*
AssetResponderMap LLActionIconAssetResponder::smPendingAssetRequests;

LLActionIconAssetResponder::LLActionIconAssetResponder(LLPanelActionIcons *actionicons, const std::string &name, LLAssetType::EType type, const std::string &hash)
   : REXActionIconResponder(actionicons, false), mName(name), mType(type), mHash(hash)
{
   smPendingAssetRequests[actionicons].push_back(this);
}

LLActionIconAssetResponder::~LLActionIconAssetResponder()
{
   if (smPendingAssetRequests.find(mIcons) != smPendingAssetRequests.end())
   {
      smPendingAssetRequests[mIcons].clear();
   }
}

void LLActionIconAssetResponder::abortRequests(LLPanelActionIcons *actionicons)
{
   AssetResponderMap::iterator iter = smPendingAssetRequests.find(actionicons);
   if (iter != smPendingAssetRequests.end())
   {
      std::list<LLActionIconAssetResponder*>::iterator responderIter = iter->second.begin();
      for ( ; responderIter != iter->second.end() ; ++responderIter)
      {
        (*responderIter)->cancel();
      }
   }
}


void LLActionIconAssetResponder::error(U32 status, const std::string& reason)
{
   llwarns << "Failed to connect to avatar storage: " << reason << llendl;
   //! \todo to error handling / reporting (popup to user about not being able to reach avatar storage server?)
}

void LLActionIconAssetResponder::result(const LLSD& content)
{
   if (mCancelRequest && mOtherAvatars.empty())
      return;

   llinfos << "Fetching asset " << mName << " from avatar storage server." << llendl;

   LLSD::Binary asset;
   if (content.has(mHash))
   {
      asset = content[mHash].asBinary();
   } else
   {
      llwarns << "Malformed reply received from avatar storage. Failed to fetch asset." << llendl;
      return;
   }


   // save skeleton binary to cache 
   if (asset.empty() == false)
   {
      REXActionIconCache::getSingleton().createAsset(mName, asset, mHash, mType);

      CacheActionIconAsset &createdAsset = REXActionIconCache::getSingleton().getAsset(mHash, mType);
      Ogre::String resourceName = mIcons->createOgreAsset(mName, createdAsset, mType);
      createdAsset.setOgreResourceName(resourceName);

      if (mCancelRequest == false)
         mIcons->assetReady(resourceName, mType);

      size_t n;
      for (n=0 ; n<mOtherAvatars.size() ; ++n)
      {
         mOtherAvatars[n]->assetReady(resourceName, mType);
      }
   } else
   {
       llwarns << "Received empty asset from avatar storage. Failed to fetch asset." << llendl;
   }
}



*/
REXActionIconCache::REXActionIconCache()
{
   llinfos << "Preparing actionicons cache..." << llendl;
   mCacheDataFile = gDirUtilp->getExpandedFilename(LL_PATH_CACHE, DEFAULT_ACTIONICONCACHE_FILE);
   const char *indexfile = gVFS->getIndexFilename();

   mCacheDataFile = constructAssetCacheFile(indexfile);

   mStorageUri = gSavedSettings.getString("AvatarStorageUrl");


   if (!loadCacheData())
      createCacheDataFile();
}

REXActionIconCache::~REXActionIconCache()
{
}

bool REXActionIconCache::loadCacheData()
{
   if (mCacheData.LoadFile(mCacheDataFile.c_str()) == false)
   {
      if (mCacheData.Error())
      {
         // Do we need a warning here? probably not
         llwarns << "Error occurred while loading actionicons asset cache data " << mCacheDataFile << ": " << mCacheData.ErrorDesc() << llendl;
      } else
      {
         llwarns << "Failed to open actionicons asset cache data " << mCacheDataFile << llendl;
      }
      return false;
   }

   if (parseCacheEntries() == false)
   {
      llwarns << "ActionIcon assets cache entries corrupted, creating new cache entries file..." << llendl; 
      return false;
   }

   return true;
}

bool REXActionIconCache::parseCacheEntries()
{
   Rex::XmlNode *root = mCacheData.RootElement();
   if (!root)
      return false;

   Rex::XmlElement *asset = root->FirstChildElement();
   while(asset)
   {
      AssetEntry entry;
      const char *name = asset->Attribute("name");
      if (!name)
         return false;
      entry.mName = std::string(name);

      const char *uid = asset->Attribute("cache");
      if (!uid)
         return false;

      LLUUID id(uid);
      entry.mUid = id;

      const char *hash = asset->Attribute("hash");
      if (!hash)
         return false;
      
      std::string hashstr(hash);
      mCacheEntries[hashstr] = entry;

      asset = asset->NextSiblingElement();
   }

   return true;
}

void REXActionIconCache::createCacheDataFile()
{
   llinfos << "Creating actionicons cache..." << llendl;

   static const char *cachetemplate =
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<!-- Avatar cache data template -->"
			"<actionicons_cache>"
			"</actionicons_cache>";

   Rex::XmlDocument doc("some");
   doc.Parse(cachetemplate);
   doc.SaveFile(mCacheDataFile.c_str());

   loadCacheData();
}

REXActionIconCache &REXActionIconCache::getSingleton()
{
   static REXActionIconCache cache;
   return cache;
}


//! Returns singleton instance pointer
REXActionIconCache *REXActionIconCache::getSingletonPtr()
{
   return &getSingleton();
}

void REXActionIconCache::requestUpdate(LLPanelActionIcons *actionicons)
{
   llassert(actionicons != 0);
   if (!actionicons)
      return;

   LLString address = actionicons->getStorageAddress();
   if (address.empty())
   {
	   llwarns << "No storage uri set for actionicons." << llendl;
	   return;
   }

   llinfos << "Requesting actionicons from uri " << address << "." << llendl;

   LLHTTPClient::ResponderPtr responder = new REXActionIconResponder(actionicons);
   LLHTTPClient::get(address, responder);
}

bool REXActionIconCache::hasAssetBase64(const std::string &hash) const
{
   if (mCacheEntries.find(hash) != mCacheEntries.end())
   {
      return true;
   }

   return false;
}
/*
bool REXActionIconCache::createAsset(const std::string &name, const LLSD::Binary &asset, const std::string &hash, LLAssetType::EType type)
{   
   if (asset.empty() || hash.empty())// || type == LLAssetType::AT_NONE)
      return false;



   llassert(validateType(type) == true);
   if (validateType(type) == false)
      return false;

   Rex::XmlElement *root = mCacheData.RootElement();
   if (!root)
   {
      // oh noes, something terrible has happened, we have no cache data
      llwarns << "No cache data file where asset could be saved!" << llendl;
      return false;
   }

   LLUUID asset_id;
   asset_id.generate();
   // Find uuid that's not in use
   while(gVFS->getExists(asset_id, type)) asset_id.generate();

   LLVFile file(gVFS, asset_id, type, LLVFile::WRITE);
   file.setMaxSize(asset.size());
   const U8 *const_pointer = &asset[0];
   file.write(const_pointer, asset.size());



   if (hasAssetBase64(hash))
   {
      // Remove old entry
      Rex::XmlElement *entry = root->FirstChildElement("Asset");
      while (entry)
      {
         Rex::XmlElement *current = entry;
         entry = entry->NextSiblingElement("Asset");

         const std::string *oldHash = current->Attribute(std::string("hash"));
         if (oldHash)
         {
            if (*oldHash == hash)
            {
               root->RemoveChild(current);
               llinfos << "Removed old cache entry for hash: " << hash << llendl;
               break;
            }
         }
      }
   }

   
   // Create new entry
   Rex::XmlElement item("Asset");
//   Rex::XmlElement item(hash);
   item.SetAttribute("cache", asset_id.asString());
   item.SetAttribute("name", name);
   item.SetAttribute("hash", hash);
   root->InsertEndChild(item);
   
   saveAssets();

   AssetEntry entry;
   entry.mName = name;
   entry.mUid = asset_id;
   mCacheEntries[hash] = entry;
   
   return true;
}

void REXActionIconCache::setStorageUri(const LLString &uri)
{
   mStorageUri = uri;
}

LLString REXActionIconCache::getStorageUri() const
{
   return mStorageUri;
}

*//*
CacheActionIconAsset &REXActionIconCache::getAsset(const std::string &hash, LLAssetType::EType type)
{
   if (mLoadedAssets.find(hash) != mLoadedAssets.end())
   {
      if (mLoadedAssets[hash].isLoaded())
         return mLoadedAssets[hash];
   } else
   {

      // Asset not in memory, lets load it from cache
      CacheActionIconAsset asset;
      mLoadedAssets[hash] = asset;
   }

   if (hasAssetBase64(hash) == false)
   {
      llwarns << "Asset not in cache" << llendl;
      return mLoadedAssets[hash];
   }
   llassert(validateType(type) == true);
   if (validateType(type) == false)
      return mLoadedAssets[hash];

   mLoadedAssets[hash].load(mCacheEntries[hash].mName, mCacheEntries[hash].mUid, type);


   return mLoadedAssets[hash];
}*/
/*
void REXActionIconCache::saveAssets() const
{
   mCacheData.SaveFile(mCacheDataFile);
}

std::string REXActionIconCache::binToString(const LLSD::Binary data) const
{
   std::string str;
   str.resize(data.size());
   size_t n;
   for (n=0 ; n<data.size() ; ++n)
      str[n] = data[n];

   return str;
}

bool REXActionIconCache::validateType(LLAssetType::EType type)
{
   if (type == LLAssetType::AT_MESH || type == LLAssetType::AT_SKELETON || type == LLAssetType::AT_MATERIAL || type == LLAssetType::AT_TEXTURE ||
      type == LLAssetType::AT_GENERIC_AVATAR || type == LLAssetType::AT_PHYSICS_CLOTH)
      return true;

   return false;
}
*/
std::string REXActionIconCache::constructAssetCacheFile(const std::string &assetIndexFile)
{
   std::string cachefile = assetIndexFile;
   cachefile.append(DEFAULT_ACTIONICONCACHE_FILE);

   return cachefile;
}

std::string REXActionIconCache::calculateHash(U8 *data, size_t dataSize)
{
   std::string hashString;
	if (dataSize)
	{
		CSHA1 Sha1;
		Sha1.Update(data, dataSize);
		Sha1.Final();
		Ogre::uint8 hash[20];
		Sha1.GetHash(hash);

		hashString = LLBase64::encode(&hash[0], 20);
	}
   return hashString;
}



