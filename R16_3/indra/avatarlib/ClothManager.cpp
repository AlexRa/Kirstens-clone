// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "ClothManager.h"


template<> Rex::ClothManager *Ogre::Singleton<Rex::ClothManager>::ms_Singleton = 0;
//const Ogre::String Rex::ClothManager::XMLDATA_RESOURCE_GROUP_NAME = "XMLData";

namespace Rex
{
   ClothManager *ClothManager::getSingletonPtr()
   {
      return ms_Singleton;
   }

   ClothManager &ClothManager::getSingleton()
   {
      assert(ms_Singleton);  
      return(*ms_Singleton);
   }

   ClothManager::ClothManager()
   {
      mResourceType = "Cloth";

      // Low, as it will reference other resources
      mLoadOrder = 30.0f;

      // Register this manager
      Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
   }

   ClothManager::~ClothManager()
   {
      // Unregister this manager
      Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
   }

   ClothPtr ClothManager::load(const Ogre::String &name, const Ogre::String &group)
   {
       ClothPtr cloth = getByName(name);

       if (cloth.isNull())
           cloth = create(name, group);

       cloth->load();
       return cloth;
   }

   Ogre::Resource *ClothManager::createImpl(const Ogre::String &name, Ogre::ResourceHandle handle, 
                                          const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
                                          const Ogre::NameValuePairList *createParams)
   {
       return new Cloth(this, name, handle, group, isManual, loader);
   }
} // namespace Rex

