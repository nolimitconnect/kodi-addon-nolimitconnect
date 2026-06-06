/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "LinuxStorageProvider.h"
#include <string.h>

#if !defined(ENABLE_NLC_PLAYER)
#include "UDevProvider.h"
#endif // defined(ENABLE_NLC_PLAYER)

#ifdef HAS_DBUS
#include "UDisksProvider.h"
#include "UDisks2Provider.h"
#endif

std::unique_ptr<IStorageProvider> IStorageProvider::CreateInstance()
{
    return std::make_unique<CLinuxStorageProvider> (CLinuxStorageProvider());
}

CLinuxStorageProvider::CLinuxStorageProvider()
{
#if defined(ENABLE_NLC_PLAYER)
    return;
#else

  m_instance = NULL;

#ifdef HAS_DBUS
  if (CUDisks2Provider::HasUDisks2())
    m_instance = new CUDisks2Provider();
  else if (CUDisksProvider::HasUDisks())
    m_instance = new CUDisksProvider();
#endif
#ifdef HAVE_LIBUDEV
  if (m_instance == NULL)
    m_instance = new CUDevProvider();
#endif

  if( !m_instance )
    m_instance = new CPosixMountProvider();
#endif // defined(ENABLE_NLC_PLAYER)
}

CLinuxStorageProvider::~CLinuxStorageProvider()
{
#if !defined(ENABLE_NLC_PLAYER)
  delete m_instance;
#endif // !defined(ENABLE_NLC_PLAYER)
}

void CLinuxStorageProvider::Initialize()
{
#if !defined(ENABLE_NLC_PLAYER)
  m_instance->Initialize();
#endif // !defined(ENABLE_NLC_PLAYER)
}

void CLinuxStorageProvider::Stop()
{
#if !defined(ENABLE_NLC_PLAYER)
     m_instance->Stop();
#endif // !defined(ENABLE_NLC_PLAYER)
}

void CLinuxStorageProvider::GetLocalDrives(VECSOURCES &localDrives)
{
    localDrives.clear();
#if !defined(ENABLE_NLC_PLAYER)
  // Home directory
  CMediaSource share;
  share.strPath = getenv("HOME");
  share.strName = g_localizeStrings.Get(21440);
  share.m_ignore = true;
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  localDrives.push_back(share);
  share.strPath = "/";
  share.strName = g_localizeStrings.Get(21453);
  localDrives.push_back(share);

  m_instance->GetLocalDrives(localDrives);
#endif // !defined(ENABLE_NLC_PLAYER)
}

void CLinuxStorageProvider::GetRemovableDrives(VECSOURCES &removableDrives)
{
  removableDrives.clear();
#if !defined(ENABLE_NLC_PLAYER)
  m_instance->GetRemovableDrives(removableDrives);
#endif // !defined(ENABLE_NLC_PLAYER)
}

bool CLinuxStorageProvider::Eject(const std::string& mountpath)
{
#if !defined(ENABLE_NLC_PLAYER)
  return m_instance->Eject(mountpath);
#else
  return true;
#endif // !defined(ENABLE_NLC_PLAYER)
}

std::vector<std::string> CLinuxStorageProvider::GetDiskUsage()
{
#if defined(ENABLE_NLC_PLAYER)
  // use the same as posix
  std::vector<std::string> result;

  char line[1024];

#if defined(TARGET_DARWIN)
  FILE* pipe = popen("df -hT ufs,cd9660,hfs,udf", "r");
#elif defined(TARGET_FREEBSD)
  FILE* pipe = popen("df -h -t ufs,cd9660,hfs,udf,zfs", "r");
#else
  FILE* pipe = popen("df -h", "r");
#endif

  static const char* excludes[] = {"rootfs","devtmpfs","tmpfs","none","/dev/loop", "udev", NULL};

  if (pipe)
  {
      while (fgets(line, sizeof(line) - 1, pipe))
      {
          bool ok=true;
          for (int i=0;excludes[i];++i)
          {
              if (strstr(line,excludes[i]))
              {
                  ok=false;
                  break;
              }
          }
          if (ok)
              result.push_back(line);
      }
      pclose(pipe);
  }

  return result;
#else
  return m_instance->GetDiskUsage();
#endif // defined(ENABLE_NLC_PLAYER)
}

bool CLinuxStorageProvider::PumpDriveChangeEvents(IStorageEventsCallback *callback)
{
#if defined(ENABLE_NLC_PLAYER)
  return false;
#else
  return m_instance->PumpDriveChangeEvents(callback);
#endif // defined(ENABLE_NLC_PLAYER)
}
