/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "config_components_kodi.h"
#include "CompileInfo.h"

#include <CoreLib/AppVersion.h>

#include <cstddef>
#include <string>
#include <algorithm>

#ifndef DATE
#ifdef __DATE__
#define DATE __DATE__
#else
#define DATE "xx/xx/xx"
#endif
#endif

#ifndef TIME
#ifdef __TIME__
#define TIME __TIME__
#else
#define TIME "xx:xx:xx"
#endif
#endif

 /* XXX Only unix build process has been tested */
#ifndef GITVERSION
#define GITVERSION ""
#endif
#ifndef GITTAG
#define GITTAG ""
#endif
#ifndef GITBRANCH
#define GITBRANCH ""
#endif

// NOTE: in order to be able to use Kodi updates the build has to be the Kodi info

int CCompileInfo::GetMajor()
{
  return 18;
}

int CCompileInfo::GetMinor()
{
  return 1;
}

const char* CCompileInfo::GetPackage()
{
    return APP_PACKAGE;
}

const char* CCompileInfo::GetClass()
{
  static std::string s_classname;
  
  if (s_classname.empty())
  {
    s_classname = CCompileInfo::GetPackage();
    std::replace(s_classname.begin(), s_classname.end(), '.', '/');
  }
  return s_classname.c_str();
}

const char* CCompileInfo::GetAppName()
{
    return APP_NAME;
}

const char* CCompileInfo::GetAppNameLowerCase()
{
    return APP_DOMAIN_NAME;
}

const char* CCompileInfo::GetSuffix()
{
  return "";
}

const char* CCompileInfo::GetSCMID()
{
  return "20190310-nogitfound";
}

const char* CCompileInfo::GetCopyrightYears()
{
    return "2005-2024";
}

std::string  CCompileInfo::GetBuildDate()
{
    const std::string bdate = "20240510";
    if( !bdate.empty() )
    {
        std::string datestamp = bdate.substr( 0, 4 ) + "-" + bdate.substr( 4, 2 ) + "-" + bdate.substr( 6, 2 );
        return datestamp;
    }

    return "1970-01-01";
}

const char* CCompileInfo::GetHomeEnvName()
{
    return "NLC_HOME";
}

const char* CCompileInfo::GetBinHomeEnvName()
{
    return "NLC_BIN_HOME";
}

const char* CCompileInfo::GetBinAddonEnvName()
{
    return "NLC_BINADDON_PATH";
}

const char* CCompileInfo::GetTempEnvName()
{
    return "NLC_TEMP";
}

const char* CCompileInfo::GetUserProfileEnvName()
{
    return "NLC_PROFILE_USERDATA";
}

std::string CCompileInfo::GetSharedLibrarySuffix()
{
#if defined(TARGET_OS_WINDOWS)
    return "dll";
#else
    return "so";
#endif // defined(TARGET_OS_WINDOWS)
}

const char* CCompileInfo::GetVersionCode()
{
    return APP_VERSION;
}

std::vector<std::string> CCompileInfo::GetWebserverExtraWhitelist()
{
    std::vector<std::string> whiteList;
    return whiteList;
}
