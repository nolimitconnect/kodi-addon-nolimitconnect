/*
 *  Copyright (C) 2013-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once
#include "config_components_kodi.h"
#if HAVE_ADDONS

#include "ContextMenuItem.h"
#include "threads/CriticalSection.h"

#include <memory>
#include <utility>
#include <vector>

namespace ADDON
{
struct AddonEvent;
class CAddonMgr;
} // namespace ADDON

#if ENABLE_PVR
namespace PVR
{
  struct PVRContextMenuEvent;
}
#endif // ENABLE_PVR

using ContextMenuView = std::vector<std::shared_ptr<const IContextMenuItem>>;

class CContextMenuManager
{
public:
  static const CContextMenuItem MAIN;
  static const CContextMenuItem MANAGE;

  explicit CContextMenuManager(ADDON::CAddonMgr& addonMgr);
  ~CContextMenuManager();
 
  void Init();
  void Deinit();

  ContextMenuView GetItems(const CFileItem& item, const CContextMenuItem& root = MAIN) const;

  ContextMenuView GetAddonItems(const CFileItem& item, const CContextMenuItem& root = MAIN) const;

private:
  CContextMenuManager(const CContextMenuManager&) = delete;
  CContextMenuManager& operator=(CContextMenuManager const&) = delete;

  bool IsVisible(
    const CContextMenuItem& menuItem,
    const CContextMenuItem& root,
    const CFileItem& fileItem) const;

#if HAVE_ADDONS
  void ReloadAddonItems();
  void OnEvent(const ADDON::AddonEvent& event);
#endif // HAVE_ADDONS
#if ENABLE_PVR
  void OnPVREvent(const PVR::PVRContextMenuEvent& event);
#endif // ENABLE_PVR

  ADDON::CAddonMgr& m_addonMgr;
  std::vector<CContextMenuItem> m_addonItems;

  mutable CCriticalSection m_criticalSection;

  std::vector<std::shared_ptr<IContextMenuItem>> m_items;
};

namespace CONTEXTMENU
{
  /*!
   * Starts the context menu loop for a file item.
   * */
bool ShowFor(const std::shared_ptr<CFileItem>& fileItem,
             const CContextMenuItem& root = CContextMenuManager::MAIN);

  /*!
   * Shortcut for continuing the context menu loop from an existing menu item.
   */
bool LoopFrom(const IContextMenuItem& menu, const std::shared_ptr<CFileItem>& fileItem);
}

#endif // HAVE_ADDONS
