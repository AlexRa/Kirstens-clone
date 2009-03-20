// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __XmlManager_h__
#define __XmlManager_h__

#include <OGRE/OgreResourceManager.h>
#include "XmlFile.h"

namespace Rex
{
   //! Resource handling of xml files, loading, unloading, parsing...
   class XmlManager : public Ogre::ResourceManager, public Ogre::Singleton<XmlManager>
   {
   public:
      XmlManager();
      virtual ~XmlManager();

      virtual XmlFilePtr load(const Ogre::String &name, const Ogre::String &group);

      static XmlManager &getSingleton();
      static XmlManager *getSingletonPtr();

      //! Ogre resource group name for xml data resources
      static const Ogre::String XMLDATA_RESOURCE_GROUP_NAME;

   protected:
      Ogre::Resource *createImpl(const Ogre::String &name, Ogre::ResourceHandle handle,
         const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader,
         const Ogre::NameValuePairList *createParams);
   };
} // namespace Rex

#endif // __XmlManager_h__
