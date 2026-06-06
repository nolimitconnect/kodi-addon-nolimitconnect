/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "config_components_kodi.h"
#if HAVE_ADDONS

#include "addons/Scraper.h"
#include "music/Album.h"
#include "utils/ScraperUrl.h"

class CXBMCTinyXML;

namespace XFILE { class CCurlFile; }

namespace MUSIC_GRABBER
{
class CMusicAlbumInfo
{
public:
  CMusicAlbumInfo() = default;
  CMusicAlbumInfo(const std::string& strAlbumInfo, const CScraperUrl& strAlbumURL);
  CMusicAlbumInfo(const std::string& strAlbum, const std::string& strArtist, const std::string& strAlbumInfo, const CScraperUrl& strAlbumURL);
  virtual ~CMusicAlbumInfo() = default;

  bool Loaded() const { return m_bLoaded; }
  void SetLoaded(bool bLoaded) { m_bLoaded = bLoaded; }
  const CAlbum &GetAlbum() const { return m_album; }
  CAlbum& GetAlbum() { return m_album; }
  void SetAlbum(CAlbum& album);
  const std::string& GetTitle2() const { return m_strTitle2; }
  void SetTitle(const std::string& strTitle) { m_album.strAlbum = strTitle; }
#if HAVE_LIB_CURL
  const CScraperUrl& GetAlbumURL() const { return m_albumURL; }
#endif // HAVE_LIB_CURL
  float GetRelevance() const { return m_relevance; }
  void SetRelevance(float relevance) { m_relevance = relevance; }

  bool Load(XFILE::CCurlFile& http, const ADDON::ScraperPtr& scraper);

protected:
  bool m_bLoaded = false;
  CAlbum m_album;
  float m_relevance = -1;
  std::string m_strTitle2;
#if HAVE_LIB_CURL
  CScraperUrl m_albumURL;
#endif // HAVE_LIB_CURL
};

}

#endif // HAVE_ADDONS
