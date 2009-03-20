// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __ClientAvatar_h__
#define __ClientAvatar_h__

#include <string>

namespace Rex
{
   class OgreAvatar;


   //! Contains functions specific to rex client
   /*! Helper class for importing/exporting avatar either from local source (f.ex. cache)
       or external source (f.ex. download from avatar storage server)
   */
   class ClientAvatar
   {
   public:
      //! default constructor
      ClientAvatar(OgreAvatar *avatar);

      ~ClientAvatar();

      //! Returns uri for the avatar storage server for this avatar
      std::string getUri() const;

   private:
      //! Ogre specific avatar
      OgreAvatar *Avatar;
   };
}

#endif //__ClientAvatar_h__
