/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>
#include <vector>

class CVariant;



class CTextureUtils
{
public:
  /*! \brief retrieve a wrapped URL for a image file
   \param image name of the file
   \param type signifies a special type of image (eg embedded video thumb, picture folder thumb)
   \param options which options we need (eg size=thumb)
   \return full wrapped URL of the image file
   */
  static std::string GetWrappedImageURL(const std::string &image, const std::string &type = "", const std::string &options = "");
  static std::string GetWrappedThumbURL(const std::string &image);

  /*! \brief Unwrap an image://<url_encoded_path> style URL
   Such urls are used for art over the webserver or other users of the VFS
   \param image url of the image
   \return the unwrapped URL, or the original URL if unwrapping is inappropriate.
   */
  static std::string UnwrapImageURL(const std::string &image);
};
