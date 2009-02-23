// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#include <string>
#include "Platform.h"

#if defined(WIN32) || defined(WIN64)

namespace Rex
{
   Platform::Platform()
   {
   }

   Platform::~Platform()
   {
   }

   std::string Platform::getFilenameFromPath(const std::string &path)
   {
      if (path.empty() == false)
      {
         size_t delim = path.find_last_of("\\");
         size_t delim_other = path.find_last_of("/");

         delim = (delim == std::string::npos) ? delim_other : delim;
         
         if (delim != std::string::npos && (delim + 1) < path.size())
         {
            delim++;
            return (path.substr(delim, path.size() - delim));
         }
      }
      return path;
   }

   std::string Platform::getExtensionFromPath(const std::string &path)
   {
      if (path.empty() == false)
      {
         size_t delim = path.find_last_of(".");
         
         if (delim != std::string::npos && (delim + 1) < path.size())
         {
            delim++;
            return (path.substr(delim, path.size() - delim));
         }
      }
      return path;
   }

   bool Platform::fileExists(const std::string &file)
   {
      FILE *f = fopen(file.c_str(), "rb");
	   if (f)
	   {
		   fclose(f);
         return true;
      }
      return false;
   }

} // namespace Rex

#endif
