/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "imagefactory.h"

#include "ServiceBroker.h"

#include "guilib/FFmpegImage.h"
#include "utils/Mime.h"

#include <mutex>

CCriticalSection ImageFactory::m_createSec;

using namespace KODI::ADDONS;

IImage* ImageFactory::CreateLoader(const std::string& strFileName)
{
  NlcUrl url(strFileName);
  return CreateLoader(url);
}

IImage* ImageFactory::CreateLoader(const NlcUrl& url)
{
  if(!url.GetFileType().empty())
    return CreateLoaderFromMimeType("image/"+url.GetFileType());

  return CreateLoaderFromMimeType(CMime::GetMimeType(url));
}

IImage* ImageFactory::CreateLoaderFromMimeType(const std::string& strMimeType)
{
  return new CFFmpegImage(strMimeType);
}
