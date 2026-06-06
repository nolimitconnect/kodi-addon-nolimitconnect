//============================================================================
// Copyright (C) 2017 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPtopUrl.h"

#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/Invite.h>
#include <CoreLib/VxDebug.h>

#include <CoreLib/VxSktUtil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
namespace
{
    const char* PROTOCOL_DELIM = "://";
    const char* COLON_DELIM = ":";
    const char* SLASH_DELIM = "/";
    const char* ONLINE_ID_DELIM = "!";
};

//============================================================================
VxPtopUrl::VxPtopUrl( std::string& url )
{
	setUrl( url );
}

//============================================================================
VxPtopUrl::VxPtopUrl( const char* urlIn )
{
    setUrl( urlIn );
}

//============================================================================
VxPtopUrl::VxPtopUrl( const VxPtopUrl& rhs )
: m_Url( rhs.m_Url )
, m_Protocol( rhs.m_Protocol )
, m_Host( rhs.m_Host )
, m_Port( rhs.m_Port )
, m_OnlineId( rhs.m_OnlineId )
, m_HostType( rhs.m_HostType )
{
}

//============================================================================
VxPtopUrl& VxPtopUrl::operator = ( const VxPtopUrl& rhs )
{
    if( this != &rhs )
    {
        m_Url = rhs.m_Url;
        m_Protocol = rhs.m_Protocol;
        m_Host = rhs.m_Host;
        m_Port = rhs.m_Port;
        m_OnlineId = rhs.m_OnlineId;
        m_HostType = rhs.m_HostType;
    }

    return *this;
}

//============================================================================
bool VxPtopUrl::operator == ( const VxPtopUrl& rhs ) const
{
    return m_OnlineId == rhs.m_OnlineId && m_Port == rhs.m_Port && m_Host == rhs.m_Host;
}

//============================================================================
std::string VxPtopUrl::stripHost( const std::string& url ) const // remove suffix host type if exists
{
    if( url.length() > 32 )
    {
        if( url[url.length() - 2] == '!')
        {
            return url.substr( 0, url.length() - 1 );
        }
    }

    return url;
}

//============================================================================
bool VxPtopUrl::isValid( bool doesNotRequireOnlineId )
{
    bool isValid = m_Port && !m_Protocol.empty() && isHostIpValid();
    if( !doesNotRequireOnlineId )
    {
        isValid &= m_OnlineId.isValid();
    }
    
    return isValid;
}

//============================================================================
bool VxPtopUrl::isHostIpValid( void )
{
    return !m_Host.empty() && !(m_Host == "0.0.0.0");
}

//============================================================================
void VxPtopUrl::setUrl( const char* urlIn )
{
    if( urlIn )
    {
        std::string url = urlIn;
        setUrl( url );
    }
}

//============================================================================
void VxPtopUrl::setUrl( std::string url )
{
    if( url.length() < 7 )
    {
        LogMsg( LOG_ERROR, "VxPtopUrl::%s invalid url length %zu", __func__,  url.length() );
        clear();
        return;
    }

    m_Url = url;
    parseHostType();

    // protocol
    size_t iReadIdx = m_Url.find( PROTOCOL_DELIM );
    if( iReadIdx != std::string::npos )
    {
        m_Protocol = m_Url.substr( 0, iReadIdx );
        iReadIdx += strlen( PROTOCOL_DELIM );
        StdStringTrim( m_Protocol );
    }
    else
    {
        iReadIdx = 0;
    }

    // host
    size_t iSlashIdx = m_Url.find( SLASH_DELIM, iReadIdx );
    if( iSlashIdx != std::string::npos )
    {
        m_Host = m_Url.substr( iReadIdx, iSlashIdx - iReadIdx );
    }
    else
    {
        m_Host = m_Url.substr( iReadIdx, m_Url.length() - iReadIdx );
    }

    m_Port = 0;
    size_t iPeriodIdx = m_Host.rfind( "." );
    if( iPeriodIdx != std::string::npos )
    {
        // host name or ipv4
        // check if has :PortNumber
        size_t iColonIdx = m_Host.rfind( COLON_DELIM );
        if( iColonIdx != std::string::npos )
        {
            std::string strHost = m_Host;
            std::string portStr = m_Host.substr( iColonIdx + 1, m_Host.length() - iColonIdx - 1 );
            m_Port = atoi( portStr.c_str() );
            if( 0 == m_Port )
            {
                m_Port = 80;
            }

            m_Host = strHost.substr( 0, iColonIdx );
        }
    }
    else
    {
        // handle IPv6 and port
        size_t iColonIdx = m_Host.rfind( COLON_DELIM );
        if( iColonIdx > 0 )
        {
            if( strchr( m_Host.c_str(), ']' ) )
            {
                // ipv6
                size_t iLeftBracketIdx = m_Host.rfind( ']' );
                if( iColonIdx != std::string::npos && iLeftBracketIdx < iColonIdx )
                {
                    std::string strHost = m_Host;
                    m_Host = strHost.substr( 0, iColonIdx );
                    if( 0 < m_Host.length() )
                    {
                        if( ( m_Host.at( 0 ) == '[' ) &&
                            ( m_Host.at( m_Host.length() - 1 ) == ']' ) )
                        {
                            m_Host = m_Host.substr( 1, iColonIdx - 2 );
                        }
                    }

                    std::string m_PortStr = strHost.substr( iColonIdx + 1, strHost.length() - iColonIdx - 1 );
                    m_Port = atoi( m_PortStr.c_str() );
                }
            }
        }
    }

    if( iSlashIdx == std::string::npos )
    {
        return;
    }

    iReadIdx = iSlashIdx;

    // read online id if exists
    std::string strOnlineId = m_Url.substr( iReadIdx + 1, m_Url.length() - (iReadIdx + 1) );
    if( VxGUID::isOnlineIdStringValid( strOnlineId.c_str() ) )
    {
        if( strOnlineId.length() == 34 )
        {
            // there is nothing past the online id but it looks valid
            m_OnlineId.fromOnlineIdString( strOnlineId.c_str() );
        }
        else if( strOnlineId.length() == 35 )
        {
            // may have a invite type character
            m_OnlineId.fromOnlineIdString( strOnlineId.c_str() );
            if( m_OnlineId.isValid() )
            {
                char suffixChar = strOnlineId[strOnlineId.length() - 1];
                if( Invite::isValidHostTypeSuffix( suffixChar ) )
                {
                    m_HostType = Invite::getHostTypeFromSuffix( suffixChar );
                }
                else
                {
                    LogMsg( LOG_ERROR, "VxPtopUrl::setUrl invalid suffix char 0x%X (%c)", suffixChar, suffixChar );
                }
            }
        }

        iReadIdx += strOnlineId.length();
        iSlashIdx = m_Url.find( SLASH_DELIM, iReadIdx );
        if( iSlashIdx == std::string::npos )
        {
            return;
        }     
    }
}

//============================================================================
bool VxPtopUrl::setUrlHostType( EHostType hostType, bool forceHostType )
{
    bool result = setUrlHostType( m_Url, hostType );
    if( result || forceHostType )
    {
        m_HostType = hostType;
    }

    return result;
}

//============================================================================
bool VxPtopUrl::setUrlHostType( std::string& url, EHostType hostType )
{
    bool result{ false };
    if( !url.empty() )
    {
        std::size_t foundPos = url.rfind( ONLINE_ID_DELIM );
        if( foundPos != std::string::npos )
        {
            if( foundPos == url.length() - 1 )
            {
                // does not have a suffix.. just append
                url += Invite::getHostTypeSuffix( hostType );
            }
            else
            {
                // replace the character one past ! online id end char
                url[foundPos + 1] = Invite::getHostTypeSuffix( hostType );
            }

            result = true;
        }
        else if( hostType == eHostTypeConnectTest ||  hostType == eHostTypeNetwork )
        {
            // network and connection test do not require online id
            url += Invite::getHostTypeSuffix( hostType );
        }
    }

    return result;
}

//============================================================================
bool VxPtopUrl::isUrlIpv4( void )
{
    return VxIsIPv4Address( m_Host.c_str() );
}

//============================================================================
bool VxPtopUrl::isUrlIpv6( void )
{
    return VxIsIPv6Address( m_Host.c_str() );
}

//============================================================================
std::string VxPtopUrl::getHostUrl( void )
{
    return m_Url;
}

//============================================================================
GroupieId VxPtopUrl::getHostGroupieId( void )
{
    if( m_OnlineId.isValid() && isHostTypeValid() )
    {
        return GroupieId( m_OnlineId, m_OnlineId, m_HostType );
    }

    GroupieId emptyId;
    return emptyId;
}

//============================================================================
void VxPtopUrl::clear( void )
{
    m_Url.clear();
    m_Protocol.clear();
    m_Host.clear();
    m_Port = 0;
    m_OnlineId.clear();
    m_HostType = eHostTypeUnknown;
}

//============================================================================
void VxPtopUrl::parseHostType( void )
{
    char hostChar = m_Url[ m_Url.length() - 1 ];
    char prevChar = m_Url[ m_Url.length() - 2 ];
    if( !Invite::isValidHostTypeSuffix( hostChar ) )
    {
        return;
    }

    EHostType hostType = Invite::getHostTypeFromSuffix( hostChar );
    if( eHostTypeConnectTest == hostType || eHostTypeNetwork == hostType )
    {
        // might or might not have online id
        if( '!' == prevChar || std::isdigit( prevChar ) )
        {
            m_HostType = hostType;
        }
    }
    else
    {
        if( '!' == prevChar )
        {
            m_HostType = hostType;
        }
    }
}
