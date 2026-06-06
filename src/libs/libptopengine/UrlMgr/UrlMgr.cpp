//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "UrlMgr.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxSktUtil.h>
#include <string.h>

//============================================================================
UrlMgr& GetUrlMgrInstance()
{
    static UrlMgr g_UrlMgr;
    return g_UrlMgr;
}

//============================================================================
void UrlInfo::updateOnlineId( VxGUID& onlineId )
{
    if( m_OnlineId == onlineId )
    {
        // no changes needed
        return;
    }

    m_OnlineId = onlineId;

    updateUrl();
}

//============================================================================
void UrlInfo::updateUrl( void )
{
    std::string url( "ptop://" );
    if( m_Url.size() > 7 && 0 == strncmp( m_Url.c_str(), "http://", 7 ) )
    {
        url = "http://";
    }

    std::string& ipAddr = m_IpAddr;
    EIpAddrType addrType = VxGetIpAddrType( ipAddr.c_str() );
    if( eIpAddrTypeIpv6 == addrType )
    {
        url += "[";
        url += ipAddr;
        url += "]";
    }
    else
    {
        url += ipAddr;
    }

    url += ":";
    url += std::to_string( m_Port );
    url += "/";
    url += m_OnlineId.toOnlineIdString();

    m_Url = url;
}

//============================================================================
bool UrlMgr::lookupOnlineId( std::string& hostUrl, VxGUID& onlineId )
{
    m_UrlMutex.lock();
    auto iter = m_UrlMap.find( hostUrl );
    if( iter != m_UrlMap.end() )
    {
        if( iter->second.m_OnlineId.isValid() )
        {
            onlineId = iter->second.m_OnlineId;
            m_UrlMutex.unlock();
            return true;
        }
    }

    m_UrlMutex.unlock();
    return false;
}

//============================================================================
void UrlMgr::setMyOnlineNodeUrl( std::string& myNodeUrl, EIpAddrType addrType )
{
    if( fillUrlInfo( myNodeUrl, m_MyUrlInfo ) )
    {        
        m_UrlMutex.lock();
        for( auto iter = m_UrlMap.begin(); iter != m_UrlMap.end(); ++iter )
        {
            if( !iter->second.m_OnlineId.isValid() )
            {
                if( iter->second.m_Port == m_MyUrlInfo.m_Port && iter->second.m_IpAddr == m_MyUrlInfo.m_IpAddr )
                {
                    iter->second.updateOnlineId( m_MyUrlInfo.m_OnlineId );
                }
            }
        }

        m_UrlMutex.unlock();
    }
}

//============================================================================
std::string UrlMgr::resolveUrl( std::string& hostUrl, EIpAddrType addrType )
{
    if( hostUrl.empty() )
    {
        return hostUrl;
    }

    std::string url;
    m_UrlMutex.lock();
    auto iter = m_UrlMap.find( hostUrl );
    if( iter != m_UrlMap.end() )
    {
        url = iter->second.m_Url;

        m_UrlMutex.unlock();
        return url;
    }

    m_UrlMutex.unlock();

    UrlInfo urlInfo;
    if( fillUrlInfo( hostUrl, urlInfo ) )
    {
        url = urlInfo.m_Url;

        m_UrlMutex.lock();
        m_UrlMap[hostUrl] = urlInfo;
        m_UrlMutex.unlock();
    }

    return url;
}

//============================================================================
void UrlMgr::updateUrlCache( std::string& hostUrlIn, VxGUID& onlineId, EIpAddrType addrType )
{
    if( !onlineId.isValid() )
    {
        LogMsg( LOG_SEVERE, "UrlMgr::updateUrlCache onlineId is INVALID" );
        return;
    }

    std::string ipAddr;
    uint16_t tcpPort{ 0 };

    bool result = VxResolveUrl( hostUrlIn, tcpPort, ipAddr, addrType );
    if( !result )
    {
        LogMsg( LOG_SEVERE, "UrlMgr::updateUrlCache url %s is INVALID", hostUrlIn.c_str() );
        return;
    }

    std::string hostUrl = "ptop://" + ipAddr + ":" + std::to_string( tcpPort );

    bool foundUrl = false;
    m_UrlMutex.lock();
    auto iter = m_UrlMap.find( hostUrl );
    if( iter != m_UrlMap.end() )
    {
        foundUrl = true;
        if( onlineId != iter->second.m_OnlineId )
        {
            iter->second.updateOnlineId( onlineId );
        }
    }

    for( auto iter = m_UrlMap.begin(); iter != m_UrlMap.end(); ++iter )
    {
        if( !iter->second.m_OnlineId.isValid() && iter->second.m_Port == tcpPort &&iter->second.m_IpAddr == ipAddr )
        {
            iter->second.updateOnlineId( onlineId );
        }
    }

    m_UrlMutex.unlock();

    if( !foundUrl )
    {
        addUrlAndOnlineId( hostUrl, onlineId, addrType );
    }
}

//============================================================================
bool UrlMgr::addUrl( std::string& hostUrl, EIpAddrType addrType )
{
    bool urlAdded{ false };
    UrlInfo urlInfo;
    if( fillUrlInfo( hostUrl, urlInfo, addrType ) )
    {
        m_UrlMutex.lock();
        m_UrlMap[hostUrl] = urlInfo;
        m_UrlMutex.unlock();
        urlAdded = true;
    }

    return urlAdded;
}

//============================================================================
bool UrlMgr::addUrlAndOnlineId( std::string& hostUrl, VxGUID& onlineId, EIpAddrType addrType )
{
    bool urlAdded{ false };
    UrlInfo urlInfo;
    if( fillUrlInfo( hostUrl, urlInfo, addrType ) )
    {
        urlInfo.updateOnlineId( onlineId );
        m_UrlMutex.lock();
        m_UrlMap[hostUrl] = urlInfo;
        m_UrlMutex.unlock();
        urlAdded = true;
    }

    return urlAdded;
}

//============================================================================
bool UrlMgr::fillUrlInfo( std::string& hostUrl, UrlInfo& urlInfo, EIpAddrType addrType )
{
    std::string strHost;
    std::string strFile;
    uint16_t tcpPort{ 0 };

    bool result = VxSplitHostAndFile( hostUrl.c_str(), strHost, strFile, tcpPort );
    if( result )
    {
        InetAddress inetAddr;
        urlInfo.m_Port = tcpPort;
        if( VxIsIPv4Address( strHost.c_str() ) )
        {
            urlInfo.m_IpAddr = strHost;
        }
        else if( VxIsIPv6Address( strHost.c_str() ) )
        {
            urlInfo.m_IpAddr = strHost;
        }
        else if( VxResolveUrl( strHost.c_str(), tcpPort, inetAddr, addrType ) )
        {
           urlInfo.m_IpAddr = inetAddr.toString();
        }
        else
        {
            return false;
        }

        EIpAddrType addrType = VxGetIpAddrType( urlInfo.m_IpAddr.c_str() );
        bool hadOnlineId = false;
        if( !strFile.empty() && strFile[0] == '!' )
        {
            // possibly has online id
            VxGUID onlineId;
            onlineId.fromOnlineIdString( strFile.c_str() );
            if( onlineId.isValid() )
            {
                urlInfo.m_OnlineId = onlineId;
                hadOnlineId = true;
            }
        }
        else if( urlInfo.m_Port == m_MyUrlInfo.m_Port && urlInfo.m_IpAddr == m_MyUrlInfo.m_IpAddr )
        {
            urlInfo.m_OnlineId = m_MyUrlInfo.m_OnlineId;
            strFile = urlInfo.m_OnlineId.toOnlineIdString();
            hadOnlineId = true;
        }

        result = VxMakePtopUrl( urlInfo.m_IpAddr, tcpPort, urlInfo.m_Url );
    }

    return result;
}
