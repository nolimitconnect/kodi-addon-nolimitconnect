/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

// NfoFile.h: interface for the CNfoFile class.
//
//////////////////////////////////////////////////////////////////////

#include "InfoScanner.h"
#include "addons/Scraper.h"
#include "utils/XBMCTinyXML.h"

#include <string>
#include <utility>
#include <vector>

namespace ADDON
{
enum class AddonType;
}

class CNfoFile
{
public:
  virtual ~CNfoFile() { Close(); }
#if HAVE_ADDONS
  CInfoScanner::INFO_TYPE Create(const std::string&,
                                 const ADDON::ScraperPtr&, int episode=-1);
#endif // HAVE_ADDONS

  template<class T>
    bool GetDetails(T& details, const char* document=NULL,
                    bool prioritise=false)
  {
    CXBMCTinyXML doc;
    if (document)
      doc.Parse(document, TIXML_ENCODING_UNKNOWN);
    else if (m_headPos < m_doc.size())
      doc.Parse(m_doc.substr(m_headPos), TIXML_ENCODING_UNKNOWN);
    else
      return false;

    return details.Load(doc.RootElement(), true, prioritise);
  }

  void Close();

 #if HAVE_ADDONS
  void SetScraperInfo(ADDON::ScraperPtr info) { m_info = std::move(info); }
  ADDON::ScraperPtr GetScraperInfo() { return m_info; }

  const CScraperUrl &ScraperUrl() const { return m_scurl; }

  static int Scrape(ADDON::ScraperPtr& scraper, CScraperUrl& url,
                    const std::string& content);

  static std::vector<ADDON::ScraperPtr> GetScrapers(ADDON::AddonType type,
                                                    const ADDON::ScraperPtr& selectedScraper);
 #endif // HAVE_ADDONS

private:
  std::string m_doc;
  size_t m_headPos = 0;
#if HAVE_ADDONS
  ADDON::ScraperPtr m_info;
  ADDON::AddonType m_type{};
  CScraperUrl m_scurl;
 #endif // HAVE_ADDONS

  int Load(const std::string&);
};
