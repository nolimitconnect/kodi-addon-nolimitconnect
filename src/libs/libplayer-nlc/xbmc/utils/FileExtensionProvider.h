/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "config_components_kodi.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#if HAVE_ADDONS
namespace ADDON
{
enum class AddonType;
class CAddonMgr;
struct AddonEvent;
}
#endif // HAVE_ADDONS

class CAdvancedSettings;

class CFileExtensionProvider
{
public:
#if HAVE_ADDONS
  CFileExtensionProvider(ADDON::CAddonMgr& addonManager);
#else
  CFileExtensionProvider();
#endif // HAVE_ADDONS
  ~CFileExtensionProvider();

  /*!
   * @brief Returns a list of picture extensions
   */
  std::string GetPictureExtensions() const;

  /*!
   * @brief Returns a list of music extensions
   */
  std::string GetMusicExtensions() const;

  /*!
   * @brief Returns a list of video extensions
   */
  std::string GetVideoExtensions() const;

  /*!
   * @brief Returns a list of subtitle extensions
   */
  std::string GetSubtitleExtensions() const;

  /*!
   * @brief Returns a list of disc stub extensions
   */
  std::string GetDiscStubExtensions() const;

  /*!
   * @brief Returns a file folder extensions
   */
  std::string GetFileFolderExtensions() const;

  /*!
   * @brief Returns whether a url protocol from add-ons use encoded hostnames
   */
  bool EncodedHostName(const std::string& protocol) const;

  /*!
   * @brief Returns true if related provider can operate related file
   *
   * @note Thought for cases e.g. by ISO, where can be a video or also a SACD.
   */
  bool CanOperateExtension(const std::string& path) const;

private:
#if HAVE_ADDONS
  std::string GetAddonExtensions(ADDON::AddonType type) const;
  std::string GetAddonFileFolderExtensions(ADDON::AddonType type) const;
  void SetAddonExtensions();
  void SetAddonExtensions(ADDON::AddonType type);

  void OnAddonEvent(const ADDON::AddonEvent& event);

  ADDON::CAddonMgr &m_addonManager;

  // File extension properties
  std::map<ADDON::AddonType, std::string> m_addonExtensions;
  std::map<ADDON::AddonType, std::string> m_addonFileFolderExtensions;
#endif // HAVE_ADDONS

    // Construction properties
  std::shared_ptr<CAdvancedSettings> m_advancedSettings;

  // Protocols from add-ons with encoded host names
  std::vector<std::string> m_encoded;
};


