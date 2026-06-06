/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MusicAlbumInfo.h"
#if HAVE_ADDONS

#include "addons/Scraper.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"

using namespace MUSIC_GRABBER;

CMusicAlbumInfo::CMusicAlbumInfo(const std::string& strAlbumInfo, const CScraperUrl& strAlbumURL)
  : m_strTitle2(strAlbumInfo) 
#if HAVE_LIB_CURL
  , m_albumURL(strAlbumURL)
#endif // HAVE_LIB_CURL
{
  m_relevance = -1;
  m_bLoaded = false;
}

CMusicAlbumInfo::CMusicAlbumInfo(const std::string& strAlbum,
                                 const std::string& strArtist,
                                 const std::string& strAlbumInfo,
                                 const CScraperUrl& strAlbumURL)
  : m_strTitle2(strAlbumInfo)
#if HAVE_LIB_CURL
  , m_albumURL(strAlbumURL)
#endif // HAVE_LIB_CURL
{
  m_album.strAlbum = strAlbum;
  //Just setting artist desc, not populating album artist credits.
  m_album.strArtistDesc = strArtist;
  m_relevance = -1;
  m_bLoaded = false;
}

void CMusicAlbumInfo::SetAlbum(CAlbum& album)
{
  m_album = album;
  m_strTitle2 = "";
  m_bLoaded = true;
}

bool CMusicAlbumInfo::Load(XFILE::CCurlFile& http, const ADDON::ScraperPtr& scraper)
{
#if HAVE_LIB_CURL
  bool fSuccess = scraper->GetAlbumDetails(http, m_albumURL, m_album);
  if (fSuccess && m_strTitle2.empty())
    m_strTitle2 = m_album.strAlbum;
  SetLoaded(fSuccess);
  return fSuccess;
#else
    return false;
#endif // HAVE_LIB_CURL
}

#endif // HAVE_ADDONS
