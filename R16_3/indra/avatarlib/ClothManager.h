// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __ClothManager_h__
#define __ClothManager_h__

#include <OGRE/OgreResourceManager.h>
#include "XmlFile.h"
#include "Cloth.h"

namespace Rex
{
   //! Resource handling of xml files, loading, unloading, parsing...
   class ClothManager : public Ogre::ResourceManager, public Ogre::Singleton<ClothManager>
   {
   public:
      ClothManager();
      virtual ~ClothManager();

      virtual ClothPtr load(const Ogre::String &name, const Ogre::String &group);

      //! Singleton accessor
      static ClothManager &getSingleton();
      //! Singleton accessor that returns a pointer
      static ClothManager *getSingletonPtr();

   protected:
      Ogre::Resource *createImpl(const Ogre::String &name, Ogre::ResourceHandle handle,
         const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader,
         const Ogre::NameValuePairList *createParams);
   };
} // namespace Rex

#endif // __ClothManager_h__
