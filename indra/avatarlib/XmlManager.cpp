// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "XmlManager.h"


template<> Rex::XmlManager *Ogre::Singleton<Rex::XmlManager>::ms_Singleton = 0;
const Ogre::String Rex::XmlManager::XMLDATA_RESOURCE_GROUP_NAME = "XMLData";

namespace Rex
{
   XmlManager *XmlManager::getSingletonPtr()
   {
      return ms_Singleton;
   }

   XmlManager &XmlManager::getSingleton()
   {
      assert(ms_Singleton);  
      return(*ms_Singleton);
   }

   XmlManager::XmlManager()
   {
      mResourceType = "XML";

      // Low, as it will reference other resources
      mLoadOrder = 30.0f;

      // Register this manager
      Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
   }

   XmlManager::~XmlManager()
   {
      // Unregister this manager
      Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
   }

   XmlFilePtr XmlManager::load(const Ogre::String &name, const Ogre::String &group)
   {
       XmlFilePtr xml = getByName(name);

       if (xml.isNull())
           xml = create(name, group);

       xml->load();
       return xml;
   }

   Ogre::Resource *XmlManager::createImpl(const Ogre::String &name, Ogre::ResourceHandle handle, 
                                          const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
                                          const Ogre::NameValuePairList *createParams)
   {
       return new XmlFile(this, name, handle, group, isManual, loader);
   }
} // namespace Rex

