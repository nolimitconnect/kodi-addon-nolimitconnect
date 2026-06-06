/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "NlcUrl.h"


#include "FileFactory.h"
#ifdef TARGET_POSIX
#include "platform/posix/filesystem/PosixFile.h"
#elif defined(TARGET_WINDOWS)
#include "platform/win32/filesystem/Win32File.h"
#ifdef TARGET_WINDOWS_STORE
#include "platform/win10/filesystem/WinLibraryFile.h"
#endif
#endif // TARGET_WINDOWS

#include "DAVFile.h"
#include "ShoutcastFile.h"

#if defined(TARGET_ANDROID)
#include "platform/android/filesystem/APKFile.h"
#endif
#include "XbtFile.h"
#include "ZipFile.h"

#if defined(TARGET_ANDROID)
#include "platform/android/filesystem/AndroidAppFile.h"
#endif
#if defined(TARGET_DARWIN_TVOS)
#include "platform/darwin/tvos/filesystem/TVOSFile.h"
#endif // TARGET_DARWIN_TVOS

#include "PipeFile.h"
#include "MusicDatabaseFile.h"
#include "VideoDatabaseFile.h"
#include "PluginFile.h"
#include "SpecialProtocolFile.h"
#include "MultiPathFile.h"

#include "ImageFile.h"
#include "ResourceFile.h"

#include "utils/log.h"

#include "utils/StringUtils.h"
#include "ServiceBroker.h"


#if HAVE_ADDONS
using namespace ADDON;
#endif // HAVE_ADDONS
using namespace XFILE;

CFileFactory::CFileFactory() = default;

CFileFactory::~CFileFactory() = default;

IFile* CFileFactory::CreateLoader(const std::string& strFileName)
{
  NlcUrl url(strFileName);
  return CreateLoader(url);
}

IFile* CFileFactory::CreateLoader(const NlcUrl& url)
{


#if defined(TARGET_ANDROID)
  if (url.IsProtocol("apk")) return new CAPKFile();
#endif
  if (url.IsProtocol("zip")) return new CZipFile();
  else if (url.IsProtocol("xbt")) return new CXbtFile();
  //else if (url.IsProtocol("musicdb")) return new CMusicDatabaseFile();
  //else if (url.IsProtocol("videodb")) return new CVideoDatabaseFile();
  else if (url.IsProtocol("plugin")) return new CPluginFile();
  else if (url.IsProtocol("library")) return nullptr;
  else if (url.IsProtocol("pvr")) return nullptr;
  else if (url.IsProtocol("special")) return new CSpecialProtocolFile();
  else if (url.IsProtocol("multipath")) return new CMultiPathFile();
  else if (url.IsProtocol("image")) return new CImageFile();
#ifdef TARGET_POSIX
  else if (url.IsProtocol("file") || url.GetProtocol().empty())
  {
#if defined(TARGET_DARWIN_TVOS)
    if (CTVOSFile::WantsFile(url))
      return new CTVOSFile();
#endif
    return new CPosixFile();
  }
#elif defined(TARGET_WINDOWS)
  else if (url.IsProtocol("file") || url.GetProtocol().empty())
  {
#ifdef TARGET_WINDOWS_STORE
    if (CWinLibraryFile::IsInAccessList(url))
      return new CWinLibraryFile();
#endif
    return new CWin32File();
  }
#endif // TARGET_WINDOWS 

#if defined(TARGET_ANDROID)
  else if (url.IsProtocol("androidapp")) return new CFileAndroidApp();
#endif
  //else if (url.IsProtocol("pipe")) return new CPipeFile();

  else if (url.IsProtocol("resource")) return new CResourceFile();
#ifdef TARGET_WINDOWS_STORE
  else if (CWinLibraryFile::IsValid(url)) return new CWinLibraryFile();
#endif



  CLog::Log(LOGWARNING, "%s - sunsupported protocol(%s) in %s", __FUNCTION__, url.GetProtocol().c_str(), url.GetRedacted().c_str());
  return NULL;
}
