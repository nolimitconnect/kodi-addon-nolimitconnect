/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <stdlib.h>

#include "DirectoryFactory.h"
#include "SpecialProtocolDirectory.h"
#include "MultiPathDirectory.h"
#include "StackDirectory.h"
#include "FileDirectoryFactory.h"
#include "PlaylistDirectory.h"
#include "MusicDatabaseDirectory.h"
#include "MusicSearchDirectory.h"
#include "VideoDatabaseDirectory.h"
#include "FavouritesDirectory.h"
#include "LibraryDirectory.h"
#include "EventsDirectory.h"
#include "AddonsDirectory.h"
#include "SourcesDirectory.h"


#include "utils/log.h"


#ifdef TARGET_POSIX
#include "platform/posix/filesystem/PosixDirectory.h"
#elif defined(TARGET_WINDOWS)
#include "platform/win32/filesystem/Win32Directory.h"
#ifdef TARGET_WINDOWS_STORE
#include "platform/win10/filesystem/WinLibraryDirectory.h"
#endif
#endif

#include "PluginDirectory.h"

#if defined(TARGET_ANDROID)
#include "platform/android/filesystem/APKDirectory.h"
#elif defined(TARGET_DARWIN_TVOS)
#include "platform/darwin/tvos/filesystem/TVOSDirectory.h"
#endif
#include "XbtDirectory.h"
#include "ZipDirectory.h"
#include "FileItem.h"
#include "NlcUrl.h"

#if defined(TARGET_ANDROID)
#include "platform/android/filesystem/AndroidAppDirectory.h"
#endif
#include "ResourceDirectory.h"
#include "ServiceBroker.h"

#include "utils/StringUtils.h"

using namespace ADDON;

using namespace XFILE;

/*!
 \brief Create a IDirectory object of the share type specified in \e strPath .
 \param strPath Specifies the share type to access, can be a share or share with path.
 \return IDirectory object to access the directories on the share.
 \sa IDirectory
 */
IDirectory* CDirectoryFactory::Create(const NlcUrl& url)
{


  CFileItem item(url.Get(), true);
  IFileDirectory* pDir=CFileDirectoryFactory::Create(url, &item);
  if (pDir)
    return pDir;


#ifdef TARGET_POSIX
  if (url.GetProtocol().empty() || url.IsProtocol("file"))
  {
#if defined(TARGET_DARWIN_TVOS)
    if (CTVOSDirectory::WantsDirectory(url))
      return new CTVOSDirectory();
#endif
    return new CPosixDirectory();
  }
#elif defined(TARGET_WINDOWS)
  if (url.GetProtocol().empty() || url.IsProtocol("file")) return new CWin32Directory();
#else
#error Local directory access is not implemented for this platform
#endif
  if (url.IsProtocol("special")) return new CSpecialProtocolDirectory();
  //if (url.IsProtocol("sources")) return new CSourcesDirectory();

  //if (url.IsProtocol("zip")) return new CZipDirectory();
  //if (url.IsProtocol("xbt")) return new CXbtDirectory();
  //if (url.IsProtocol("multipath")) return new CMultiPathDirectory();
  //if (url.IsProtocol("stack")) return new CStackDirectory();
 /* if (url.IsProtocol("playlistmusic")) return new CPlaylistDirectory();
  if (url.IsProtocol("playlistvideo")) return new CPlaylistDirectory();*/
  //if (url.IsProtocol("musicdb")) return new CMusicDatabaseDirectory();
  //if (url.IsProtocol("musicsearch")) return new CMusicSearchDirectory();
  //if (url.IsProtocol("videodb")) return new CVideoDatabaseDirectory();
  //if (url.IsProtocol("library")) return new CLibraryDirectory();
  //if (url.IsProtocol("favourites")) return new CFavouritesDirectory();
#if defined(TARGET_ANDROID)
  if (url.IsProtocol("androidapp")) return new CAndroidAppDirectory();
#endif

  if (url.IsProtocol("resource")) return new CResourceDirectory();
  //if (url.IsProtocol("events")) return new CEventsDirectory();
#ifdef TARGET_WINDOWS_STORE
  if (CWinLibraryDirectory::IsValid(url)) return new CWinLibraryDirectory();
#endif


  CLog::Log(LOGWARNING, "%s - unsupported protocol(%s) in %s", __FUNCTION__,  url.GetProtocol().c_str(), url.GetRedacted().c_str() );
  return NULL;
}

