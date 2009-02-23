// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __Platform_h__
#define __Platform_h__


#if defined(WIN32) || defined(WIN64)
#include "PlatformWin.h"
//#endif

namespace Rex
{
   //! Contains platform specific functions
   /*! Only used by avatar generator, not used by rex client
   */
//   #if defined(WIN32) || defined(WIN64)
   class Platform : public PlatformWin
//   #endif
   {
   public:
      //! constructor
      Platform();

      //! destructor
      virtual ~Platform();

      //! Returns filename from path
      static std::string getFilenameFromPath(const std::string &path);

      //! Returns file extensions (no dots) from file path
      static std::string getExtensionFromPath(const std::string &path);

      //! Returns true if specified file exists in the file system, false otherwise
      static bool fileExists(const std::string &file);
   };
} // namespace Rex
#endif

#if LL_LINUX
#warning *** Platform for Linux not yet implemented! ***
#include "llerror.h"
namespace Rex
{
   class Platform
   {
   public:
      //! constructor
      Platform() { };

      //! destructor
      virtual ~Platform() { };

      //! Returns filename from path
      static std::string getFilenameFromPath(const std::string &path) { llerrs << "Platform::getFFP" << llendl; };

      //! Returns file extensions (no dots) from file path
      static std::string getExtensionFromPath(const std::string &path) { llerrs << "Platform::getExtFP" << llendl; };

      //! Returns true if specified file exists in the file system, false otherwise
      static bool fileExists(const std::string &file) { llerrs << "Platform::fE" << llendl; };

      //! Puts current thread to sleep for ms milliseconds
      static void sleep(int ms) { llerrs << "Platform::s" << llendl; };

      //! Returns path to OS data folder
      static std::string getOSDataFolder() { llerrs << "Platform::getODF" << llendl; };

      //! Opens file dialog and returns selected filename
      /*!
          \param filter Filter for different filetypes
      */
      static std::string getOpenFilename(const char *filter) { llerrs << "Platform::getOpenF" << llendl; };

      //! Opens file save dialog and returns filename
      /*!
          \param filter Filter for different filetypes
      */
      static std::string getSaveFilename(const char *filter, const char *defaultExtension = 0) { llerrs << "Platform::getSaveF" << llendl; };
   };
} // namespace Rex

#endif // LL_LINUX


#endif // __Platform_h__


