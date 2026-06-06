/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once
#include "config_components_kodi.h"
#include "utils/GlobalsHandling.h"

#include <memory>
#include <string>

namespace ANNOUNCEMENT
{
  class CAnnouncementManager;
}

namespace MEDIA_DETECT
{
class CDetectDVDMedia;
}

namespace PLAYLIST
{
  class CPlayListPlayer;
}

namespace KODI
{
namespace MESSAGING
{
class CApplicationMessenger;
}
} // namespace KODI

class CAppParams;
template<class T>
class CComponentContainer;
class CContextMenuManager;
class XBPython;
class CDataCacheCore;
class IAE;
class IApplicationComponent;
class CFavouritesService;
class CInputManager;
class CFileExtensionProvider;
class CNetworkBase;
class CWinSystemBase;
class CRenderSystemBase;
class CPowerManager;
class CWeatherManager;
class CPlayerCoreFactory;
class CDatabaseManager;
class CEventLog;
class CGUIComponent;
class CAppInboundProtocol;
class CSettingsComponent;
class CDecoderFilterManager;
class CMediaManager;
class CCPUInfo;
class CLog;
class CPlatform;
class CTextureCache;
class CJobManager;
class CKeyboardLayoutManager;

namespace WSDiscovery
{
class IWSDiscovery;
}

namespace KODI
{
namespace ADDONS
{
class CExtsMimeSupportList;
}


} // namespace KODI



class CServiceBroker
{
public:
  CServiceBroker();
  ~CServiceBroker();

  static CLog& GetLogging();
  static CLog& GetLogging( std::string logModule );
  static void CreateLogging();
  static bool IsLoggingUp();
  static void DestroyLogging();

  static std::shared_ptr<ANNOUNCEMENT::CAnnouncementManager> GetAnnouncementManager();
  static void RegisterAnnouncementManager(
      std::shared_ptr<ANNOUNCEMENT::CAnnouncementManager> announcementManager);
  static void UnregisterAnnouncementManager();

  static CDataCacheCore& GetDataCacheCore();
  static PLAYLIST::CPlayListPlayer& GetPlaylistPlayer();


  //static CInputManager& GetInputManager();
  static CFileExtensionProvider &GetFileExtensionProvider();

  static bool IsServiceManagerUp();
  

  static CPlayerCoreFactory &GetPlayerCoreFactory();

  static CEventLog* GetEventLog();
  static CMediaManager& GetMediaManager();
  static CComponentContainer<IApplicationComponent>& GetAppComponents();

  static CGUIComponent* GetGUI();
  static void RegisterGUI(CGUIComponent *gui);
  static void UnregisterGUI();

  static void RegisterSettingsComponent(const std::shared_ptr<CSettingsComponent>& settings);
  static void UnregisterSettingsComponent();
  static std::shared_ptr<CSettingsComponent> GetSettingsComponent();

  static void RegisterWinSystem(CWinSystemBase *winsystem);
  static void UnregisterWinSystem();
  static CWinSystemBase* GetWinSystem();
  static CRenderSystemBase* GetRenderSystem();

  static IAE* GetActiveAE();
  static void RegisterAE(IAE *ae);
  static void UnregisterAE();

  static std::shared_ptr<CAppInboundProtocol> GetAppPort();
  static void RegisterAppPort(std::shared_ptr<CAppInboundProtocol> port);
  static void UnregisterAppPort();

  static void RegisterDecoderFilterManager(CDecoderFilterManager* manager);
  static CDecoderFilterManager* GetDecoderFilterManager();

  static std::shared_ptr<CCPUInfo> GetCPUInfo();
  static void RegisterCPUInfo(std::shared_ptr<CCPUInfo> cpuInfo);
  static void UnregisterCPUInfo();

  static void RegisterTextureCache(const std::shared_ptr<CTextureCache>& cache);
  static void UnregisterTextureCache();
  static std::shared_ptr<CTextureCache> GetTextureCache();

  static void RegisterJobManager(const std::shared_ptr<CJobManager>& jobManager);
  static void UnregisterJobManager();
  static std::shared_ptr<CJobManager> GetJobManager();

  static void RegisterAppMessenger(
      const std::shared_ptr<KODI::MESSAGING::CApplicationMessenger>& appMessenger);
  static void UnregisterAppMessenger();
  static std::shared_ptr<KODI::MESSAGING::CApplicationMessenger> GetAppMessenger();


  std::unique_ptr<CLog> m_logging;
  std::shared_ptr<ANNOUNCEMENT::CAnnouncementManager> m_pAnnouncementManager;
  CGUIComponent* m_pGUI = nullptr;
  CWinSystemBase* m_pWinSystem = nullptr;
  IAE* m_pActiveAE = nullptr;
  std::shared_ptr<CAppInboundProtocol> m_pAppPort;
  std::shared_ptr<CSettingsComponent> m_pSettingsComponent;
  CDecoderFilterManager* m_decoderFilterManager = nullptr;
  std::shared_ptr<CCPUInfo> m_cpuInfo;
  std::shared_ptr<CTextureCache> m_textureCache;
  std::shared_ptr<CJobManager> m_jobManager;
  std::shared_ptr<KODI::MESSAGING::CApplicationMessenger> m_appMessenger;

};

CServiceBroker& GetServiceBrokerInstance();

#define g_serviceBroker GetServiceBrokerInstance()


