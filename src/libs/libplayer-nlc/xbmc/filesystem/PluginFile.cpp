/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "PluginFile.h"

#include "NlcUrl.h"

using namespace XFILE;

CPluginFile::CPluginFile(void) : COverrideFile(false)
{
}

CPluginFile::~CPluginFile(void) = default;

bool CPluginFile::Open(const NlcUrl& url)
{
  return false;
}

bool CPluginFile::Exists(const NlcUrl& url)
{
  return true;
}

int CPluginFile::Stat(const NlcUrl& url, struct __stat64* buffer)
{
  return -1;
}

int CPluginFile::Stat(struct __stat64* buffer)
{
  return -1;
}

bool CPluginFile::OpenForWrite(const NlcUrl& url, bool bOverWrite)
{
  return false;
}

bool CPluginFile::Delete(const NlcUrl& url)
{
  return false;
}

bool CPluginFile::Rename(const NlcUrl& url, const NlcUrl& urlnew)
{
  return false;
}

std::string CPluginFile::TranslatePath(const NlcUrl& url)
{
  return url.Get();
}
