/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "config_components_kodi.h"

#include <memory>
#include <string>
#include "platform/Platform.h"

class CAppParamParser;

namespace PLAYLIST
{
  class CPlayListPlayer;
}

class CContextMenuManager;

class CDataCacheCore;
class CFavouritesService;
class CNetworkBase;
class CWinSystemBase;
class CPowerManager;
class CWeatherManager;

namespace KODI
{
namespace ADDONS
{
class CExtsMimeSupportList;
}

} // namespace KODI

namespace MEDIA_DETECT
{
class CDetectDVDMedia;
}

class CInputManager;
class CFileExtensionProvider;
class CPlayerCoreFactory;
class CDatabaseManager;
class CProfileManager;
class CEventLog;
class CMediaManager;

class CServiceManager
{
public:
  CServiceManager();
  ~CServiceManager();

  bool InitForTesting();
  bool InitStageOne();
  bool InitStageTwo();
  bool InitStageThree(const std::shared_ptr<CProfileManager>& profileManager);
  void DeinitTesting();
  void DeinitStageThree();
  void DeinitStageTwo();
  void DeinitStageOne();

  bool HasPlayerFactory() { return m_playerCoreFactory != nullptr; }

  CContextMenuManager& GetContextMenuManager();
  CDataCacheCore& GetDataCacheCore();
  /**\brief Get the platform object. This is save to be called after Init1() was called
   */
  CPlatform& GetPlatform();


  PLAYLIST::CPlayListPlayer& GetPlaylistPlayer();
  int init_level = 0;

  //CInputManager &GetInputManager();
  CFileExtensionProvider &GetFileExtensionProvider();

  CPlayerCoreFactory &GetPlayerCoreFactory();

  CMediaManager& GetMediaManager();

  int GetInitLevel() { return init_level; }

protected:
  struct delete_dataCacheCore
  {
    void operator()(CDataCacheCore *p) const;
  };

  struct delete_contextMenuManager
  {
    void operator()(CContextMenuManager *p) const;
  };


  std::unique_ptr<CContextMenuManager, delete_contextMenuManager> m_contextMenuManager;
  std::unique_ptr<CDataCacheCore, delete_dataCacheCore> m_dataCacheCore;
  std::unique_ptr<CPlatform> m_Platform;
  std::unique_ptr<PLAYLIST::CPlayListPlayer> m_playlistPlayer;
  //std::unique_ptr<CInputManager> m_inputManager;
  std::unique_ptr<CFileExtensionProvider> m_fileExtensionProvider;


  std::unique_ptr<CPlayerCoreFactory> m_playerCoreFactory;

  std::unique_ptr<CMediaManager> m_mediaManager;

};


