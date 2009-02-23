// Copyright (c) 2007-2008 LudoCraft
// For conditions of distribution and use, see copyright notice in license.txt

#if defined(WIN32) || defined(WIN64)

#include "PlatformWin.h"
#include <Commdlg.h>
#include <Shlobj.h>

namespace Rex
{

   std::string PlatformWin::getOSDataFolder()
   {
//      wchar_t szPath[MAX_PATH];
      char szPath[MAX_PATH];
      if(SHGetFolderPath(NULL, 
                             CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
                             NULL, 
                             0, 
                             szPath) == S_OK) 
      {
         return std::string(szPath);
      }

      return std::string();
   }

   std::string PlatformWin::getOpenFilename(const char *filter)
//   std::wstring PlatformWin::getOpenFilename(const wchar_t *filter)
   {
      char path[MAX_PATH];
//      wchar_t path[MAX_PATH];
      path[0] = '\0';

      int flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

      OPENFILENAMEA lpofn;
      memset(&lpofn, 0, sizeof(OPENFILENAME));
   //   ZeroMemory(&lpofn, sizeof(lpofn));
      lpofn.lStructSize = sizeof(OPENFILENAME);
      lpofn.hwndOwner = 0;
      lpofn.hInstance = 0;
      lpofn.lpstrFile = path;
      lpofn.nMaxFile = sizeof(path);
      lpofn.Flags = flags;
      lpofn.lpstrFilter = filter;
      
      char currentDirectory[MAX_PATH];
//      wchar_t currentDirectory[MAX_PATH];
      GetCurrentDirectoryA(MAX_PATH, currentDirectory);
      if (GetOpenFileNameA(&lpofn))
      {
         SetCurrentDirectoryA(currentDirectory);
         return(std::string(lpofn.lpstrFile));
      }

      return std::string();
   }

   std::string PlatformWin::getSaveFilename(const char *filter, const char *defaultExtension)
   {
      char path[MAX_PATH];
      path[0] = '\0';

      int flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

      OPENFILENAME lpofn;
      memset(&lpofn, 0, sizeof(OPENFILENAME));
   //   ZeroMemory(&lpofn, sizeof(lpofn));
      lpofn.lStructSize = sizeof(OPENFILENAME);
      lpofn.hwndOwner = 0;
      lpofn.hInstance = 0;
      lpofn.lpstrFile = path;
      lpofn.nMaxFile = sizeof(path);
      lpofn.Flags = flags;
      lpofn.lpstrFilter = filter;
      lpofn.lpstrDefExt = defaultExtension;
      
      char currentDirectory[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, currentDirectory);
      if (GetSaveFileName(&lpofn))
      {
         SetCurrentDirectory(currentDirectory);
         return(std::string(lpofn.lpstrFile));
      }

      return std::string();
   }
} //namespace Rex

#endif // WIN32
