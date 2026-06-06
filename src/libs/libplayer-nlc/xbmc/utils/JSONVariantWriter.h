/*
 *  Copyright (C) 2015-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "config_components_kodi.h"
#if ENABLE_JSON

#include <string>

class CVariant;

class CJSONVariantWriter
{
public:
  CJSONVariantWriter() = delete;

  static bool Write(const CVariant &value, std::string& output, bool compact);
};

#endif // ENABLE_JSON
