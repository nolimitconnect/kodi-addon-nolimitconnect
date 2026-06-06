/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaManager.h"

#include "FileItem.h"
#include "ServiceBroker.h"
#include "guilib/GUIComponent.h"
#include "guilib/LocalizeStrings.h"
#include "NlcUrl.h"
#include "utils/URIUtils.h"

#include <mutex>
#ifdef TARGET_WINDOWS
#include "platform/win32/WIN32Util.h"
#include "utils/CharsetConverter.h"
#endif
#include "guilib/GUIWindowManager.h"

#include "AutorunMediaJob.h"
#include "GUIUserMessages.h"

#include "messaging/helpers/DialogOKHelper.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSourceSettings.h"
#include "settings/Settings.h"
#include "settings/SettingsComponent.h"
#include "utils/FileUtils.h"
#include "utils/JobManager.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXML.h"
#include "utils/XMLUtils.h"
#include "utils/log.h"

#include <string>
#include <vector>

#ifdef HAS_OPTICAL_DRIVE
using namespace MEDIA_DETECT;
#endif

const char MEDIA_SOURCES_XML[] = { "special://profile/mediasources.xml" };

CMediaManager::CMediaManager()
{
  m_bhasoptical = false;
}

void CMediaManager::Stop()
{
  if (m_platformStorage)
    m_platformStorage->Stop();

  m_platformStorage.reset();
}

void CMediaManager::Initialize()
{
  if (!m_platformStorage)
  {
    m_platformStorage = IStorageProvider::CreateInstance();
  }

  m_platformStorage->Initialize();
}

bool CMediaManager::LoadSources()
{
  // clear our location list
  m_locations.clear();

  // load xml file...
  CXBMCTinyXML xmlDoc;
  if ( !xmlDoc.LoadFile( MEDIA_SOURCES_XML ) )
    return false;

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  if (!pRootElement || StringUtils::CompareNoCase(pRootElement->Value(), "mediasources") != 0)
  {
    CLog::Log(LOGERROR, "Error loading %s, Line %d (%s)", MEDIA_SOURCES_XML, xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
    return false;
  }

  // load the <network> block
  TiXmlNode *pNetwork = pRootElement->FirstChild("network");
  if (pNetwork)
  {
    TiXmlElement *pLocation = pNetwork->FirstChildElement("location");
    while (pLocation)
    {
      CNetworkLocation location;
      pLocation->Attribute("id", &location.id);
      if (pLocation->FirstChild())
      {
        location.path = pLocation->FirstChild()->Value();
        m_locations.push_back(location);
      }
      pLocation = pLocation->NextSiblingElement("location");
    }
  }
  LoadAddonSources();
  return true;
}

bool CMediaManager::SaveSources()
{
  CXBMCTinyXML xmlDoc;
  TiXmlElement xmlRootElement("mediasources");
  TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
  if (!pRoot) return false;

  TiXmlElement networkNode("network");
  TiXmlNode *pNetworkNode = pRoot->InsertEndChild(networkNode);
  if (pNetworkNode)
  {
    for (std::vector<CNetworkLocation>::iterator it = m_locations.begin(); it != m_locations.end(); ++it)
    {
      TiXmlElement locationNode("location");
      locationNode.SetAttribute("id", (*it).id);
      TiXmlText value((*it).path);
      locationNode.InsertEndChild(value);
      pNetworkNode->InsertEndChild(locationNode);
    }
  }
  return xmlDoc.SaveFile(MEDIA_SOURCES_XML);
}

void CMediaManager::GetLocalDrives(VECSOURCES &localDrives, bool includeQ)
{
  std::unique_lock<CCriticalSection> lock(m_CritSecStorageProvider);
  m_platformStorage->GetLocalDrives(localDrives);
}

void CMediaManager::GetRemovableDrives(VECSOURCES &removableDrives)
{
  std::unique_lock<CCriticalSection> lock(m_CritSecStorageProvider);
  if (m_platformStorage)
    m_platformStorage->GetRemovableDrives(removableDrives);
}

void CMediaManager::GetNetworkLocations(VECSOURCES &locations, bool autolocations)
{
  for (unsigned int i = 0; i < m_locations.size(); i++)
  {
    CMediaSource share;
    share.strPath = m_locations[i].path;
    NlcUrl url(share.strPath);
    share.strName = url.GetWithoutUserDetails();
    locations.push_back(share);
  }
  if (autolocations)
  {
    CMediaSource share;
    share.m_ignore = true;
  }
}

bool CMediaManager::AddNetworkLocation(const std::string &path)
{
  CNetworkLocation location;
  location.path = path;
  location.id = (int)m_locations.size();
  m_locations.push_back(location);
  return SaveSources();
}

bool CMediaManager::HasLocation(const std::string& path) const
{
  for (unsigned int i=0;i<m_locations.size();++i)
  {
    if (URIUtils::CompareWithoutSlashAtEnd(m_locations[i].path, path))
      return true;
  }

  return false;
}


bool CMediaManager::RemoveLocation(const std::string& path)
{
  for (unsigned int i=0;i<m_locations.size();++i)
  {
    if (URIUtils::CompareWithoutSlashAtEnd(m_locations[i].path, path))
    {
      // prompt for sources, remove, cancel,
      m_locations.erase(m_locations.begin()+i);
      return SaveSources();
    }
  }

  return false;
}

bool CMediaManager::SetLocationPath(const std::string& oldPath, const std::string& newPath)
{
  for (unsigned int i=0;i<m_locations.size();++i)
  {
    if (URIUtils::CompareWithoutSlashAtEnd(m_locations[i].path, oldPath))
    {
      m_locations[i].path = newPath;
      return SaveSources();
    }
  }

  return false;
}

void CMediaManager::LoadAddonSources() const
{
  if (CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_bVirtualShares)
  {
    CMediaSourceSettings::GetInstance().AddShare("video", GetRootAddonTypeSource("video"));
    CMediaSourceSettings::GetInstance().AddShare("programs", GetRootAddonTypeSource("programs"));
    CMediaSourceSettings::GetInstance().AddShare("pictures", GetRootAddonTypeSource("pictures"));
    CMediaSourceSettings::GetInstance().AddShare("music", GetRootAddonTypeSource("music"));
    CMediaSourceSettings::GetInstance().AddShare("games", GetRootAddonTypeSource("games"));
  }
}

CMediaSource CMediaManager::GetRootAddonTypeSource(const std::string& type) const
{
  if (type == "programs" || type == "myprograms")
  {
    return ComputeRootAddonTypeSource("executable", g_localizeStrings.Get(1043),
                                      "DefaultAddonProgram.png");
  }
  else if (type == "video" || type == "videos")
  {
    return ComputeRootAddonTypeSource("video", g_localizeStrings.Get(1037),
                                      "DefaultAddonVideo.png");
  }
  else if (type == "music")
  {
    return ComputeRootAddonTypeSource("audio", g_localizeStrings.Get(1038),
                                      "DefaultAddonMusic.png");
  }
  else if (type == "pictures")
  {
    return ComputeRootAddonTypeSource("image", g_localizeStrings.Get(1039),
                                      "DefaultAddonPicture.png");
  }
  else if (type == "games")
  {
    return ComputeRootAddonTypeSource("game", g_localizeStrings.Get(35049), "DefaultAddonGame.png");
  }
  else
  {
    CLog::LogF(LOGERROR, "Invalid type %s provided", type.c_str());
    return {};
  }
}

CMediaSource CMediaManager::ComputeRootAddonTypeSource(const std::string& type,
                                                       const std::string& label,
                                                       const std::string& thumb) const
{
  CMediaSource source;
  source.strPath = "addons://sources/" + type + "/";
  source.strName = label;
  source.m_strThumbnailImage = thumb;
  source.m_iDriveType = CMediaSource::SOURCE_TYPE_VPATH;
  source.m_ignore = true;
  return source;
}

void CMediaManager::AddAutoSource(const CMediaSource &share, bool bAutorun)
{
  CMediaSourceSettings::GetInstance().AddShare("files", share);
  CMediaSourceSettings::GetInstance().AddShare("video", share);
  CMediaSourceSettings::GetInstance().AddShare("pictures", share);
  CMediaSourceSettings::GetInstance().AddShare("music", share);
  CMediaSourceSettings::GetInstance().AddShare("programs", share);
  //CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_SOURCES);
  //CGUIComponent *gui = CServiceBroker::GetGUI();
  //if (gui)
  //  gui->GetWindowManager().SendThreadMessage( msg );

}

void CMediaManager::RemoveAutoSource(const CMediaSource &share)
{
  CMediaSourceSettings::GetInstance().DeleteSource("files", share.strName, share.strPath, true);
  CMediaSourceSettings::GetInstance().DeleteSource("video", share.strName, share.strPath, true);
  CMediaSourceSettings::GetInstance().DeleteSource("pictures", share.strName, share.strPath, true);
  CMediaSourceSettings::GetInstance().DeleteSource("music", share.strName, share.strPath, true);
  CMediaSourceSettings::GetInstance().DeleteSource("programs", share.strName, share.strPath, true);
  //CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_SOURCES);
  //CServiceBroker::GetGUI()->GetWindowManager().SendThreadMessage( msg );

}

/////////////////////////////////////////////////////////////
// AutoSource status functions:
//! @todo translate cdda://<device>/

std::string CMediaManager::TranslateDevicePath(const std::string& devicePath, bool bReturnAsDevice)
{
  std::unique_lock<CCriticalSection> waitLock(m_muAutoSource);
  std::string strDevice = devicePath;
  // fallback for cdda://local/ and empty devicePath


#ifdef TARGET_WINDOWS
  if(!m_bhasoptical)
    return "";

  if(bReturnAsDevice == false)
    StringUtils::Replace(strDevice, "\\\\.\\","");
  else if(!strDevice.empty() && strDevice[1]==':')
    strDevice = StringUtils::Format("\\\\.\\%c:", strDevice[0]);

  URIUtils::RemoveSlashAtEnd(strDevice);
#endif
  return strDevice;
}

bool CMediaManager::IsDiscInDrive(const std::string& devicePath)
{
  return false;
}

bool CMediaManager::IsAudio(const std::string& devicePath)
{
  return false;
}

bool CMediaManager::HasOpticalDrive()
{
  return false;
}

DriveState CMediaManager::GetDriveStatus(const std::string& devicePath)
{
  return DriveState::NOT_READY;
}

void CMediaManager::SetHasOpticalDrive(bool bstatus)
{
  std::unique_lock<CCriticalSection> waitLock(m_muAutoSource);
  m_bhasoptical = bstatus;
}

bool CMediaManager::Eject(const std::string& mountpath)
{
  std::unique_lock<CCriticalSection> lock(m_CritSecStorageProvider);
  return m_platformStorage->Eject(mountpath);
}

void CMediaManager::EjectTray( const bool bEject, const char cDriveLetter )
{
}

void CMediaManager::CloseTray(const char cDriveLetter)
{
}

void CMediaManager::ToggleTray(const char cDriveLetter)
{
}

void CMediaManager::ProcessEvents()
{
  std::unique_lock<CCriticalSection> lock(m_CritSecStorageProvider);
  if (m_platformStorage->PumpDriveChangeEvents(this))
  {
    
    //CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_UPDATE_SOURCES);
    //CServiceBroker::GetGUI()->GetWindowManager().SendThreadMessage(msg);
  }
}

std::vector<std::string> CMediaManager::GetDiskUsage()
{
  std::unique_lock<CCriticalSection> lock(m_CritSecStorageProvider);
  return m_platformStorage->GetDiskUsage();
}

void CMediaManager::OnStorageAdded(const MEDIA_DETECT::STORAGE::StorageDevice& device)
{
}

void CMediaManager::OnStorageSafelyRemoved(const MEDIA_DETECT::STORAGE::StorageDevice& device)
{
  //CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(13023),
  //                                      device.label, TOAST_DISPLAY_TIME, false);
}

void CMediaManager::OnStorageUnsafelyRemoved(const MEDIA_DETECT::STORAGE::StorageDevice& device)
{
  //CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Warning, g_localizeStrings.Get(13022),
  //                                      device.label);
}

bool CMediaManager::playStubFile(const CFileItem& item)
{
  //// Figure out Lines 1 and 2 of the dialog
  //std::string strLine1, strLine2;

  //// use generic message by default
  //strLine1 = g_localizeStrings.Get(435).c_str();
  //strLine2 = g_localizeStrings.Get(436).c_str();

  //CXBMCTinyXML discStubXML;
  //if (discStubXML.LoadFile(item.GetPath()))
  //{
  //  TiXmlElement* pRootElement = discStubXML.RootElement();
  //  if (!pRootElement || StringUtils::CompareNoCase(pRootElement->Value(), "discstub") != 0)
  //    CLog::Log(LOGINFO, "No <discstub> node found for %s. Using default info dialog message",
  //              item.GetPath().c_str());
  //  else
  //  {
  //    XMLUtils::GetString(pRootElement, "title", strLine1);
  //    XMLUtils::GetString(pRootElement, "message", strLine2);
  //    // no title? use the label of the CFileItem as line 1
  //    if (strLine1.empty())
  //      strLine1 = item.GetLabel();
  //  }
  //}

  //if (HasOpticalDrive())
  //{

  //}
  //else
  //{
  //  KODI::MESSAGING::HELPERS::ShowOKDialogText(strLine1, strLine2);
  //}
  return true;
}
