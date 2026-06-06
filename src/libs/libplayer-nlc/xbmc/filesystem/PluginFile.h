/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "filesystem/OverrideFile.h"

namespace XFILE
{
class CPluginFile : public COverrideFile
{
public:
  CPluginFile(void);
  ~CPluginFile(void) override;
  bool Open(const NlcUrl& url) override;
  bool Exists(const NlcUrl& url) override;
  int Stat(const NlcUrl& url, struct __stat64* buffer) override;
  int Stat(struct __stat64* buffer) override;
  bool OpenForWrite(const NlcUrl& url, bool bOverWrite = false) override;
  bool Delete(const NlcUrl& url) override;
  bool Rename(const NlcUrl& url, const NlcUrl& urlnew) override;

protected:
  std::string TranslatePath(const NlcUrl& url) override;
};
} // namespace XFILE
