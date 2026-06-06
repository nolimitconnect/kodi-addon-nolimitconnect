/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "TextureUtils.h"

#include "NlcUrl.h"
#include "StringUtils.h"

std::string CTextureUtils::GetWrappedImageURL(const std::string &image, const std::string &type, const std::string &options)
{
  if (StringUtils::StartsWith(image, "image://"))
    return image; // already wrapped

  NlcUrl url;
  url.SetProtocol("image");
  url.SetUserName(type);
  url.SetHostName(image);
  if (!options.empty())
  {
    url.SetFileName("transform");
    url.SetOptions("?" + options);
  }
  return url.Get();
}

std::string CTextureUtils::GetWrappedThumbURL(const std::string &image)
{
  return GetWrappedImageURL(image, "", "size=thumb");
}

std::string CTextureUtils::UnwrapImageURL(const std::string &image)
{
  if (StringUtils::StartsWith(image, "image://"))
  {
    NlcUrl url(image);
    if (url.GetPwdUserName().empty() && url.GetOptions().empty())
      return url.GetHostName();
  }
  return image;
}
