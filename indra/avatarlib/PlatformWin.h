// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __PlatformWin_h__
#define __PlatformWin_h__

#if defined(WIN32) || defined(WIN64)

#include <string>
#include <Windows.h>

namespace Rex
{
   //! Contains implementations of platform specific functions
   class PlatformWin
   {
   public:
      //! Puts current thread to sleep for ms milliseconds
      static void sleep(int ms) { Sleep(ms); }

      //! Returns path to OS data folder
      static std::string getOSDataFolder();

      //! Opens file dialog and returns selected filename
      /*!
          \param filter Filter for different filetypes
      */
      static std::string getOpenFilename(const char *filter);

      //! Opens file save dialog and returns filename
      /*!
          \param filter Filter for different filetypes
      */
      static std::string getSaveFilename(const char *filter, const char *defaultExtension = 0);
//      static std::wstring getOpenFilename(const wchar_t *filter);
   };
} // namespace Rex

#endif // WIN32
#endif // __PlatformWin_h__



