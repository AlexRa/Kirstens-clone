// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include "ClientAvatar.h"
#include "OgreAvatar.h"

namespace Rex
{
   ClientAvatar::ClientAvatar(OgreAvatar *avatar) : Avatar(avatar)
   {
      assert(Avatar);
   }

   ClientAvatar::~ClientAvatar()
   {
   }

   std::string ClientAvatar::getUri() const
   {
      // full circle: back to hardcoded avatar uri
      return std::string("http://192.168.1.175:10000/avatar/0fa6c8c545224e8db0286c34c3238583");
   }


}

