//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "Invite.h"
#include <CoreLib/VxParse.h>
#include <CoreLib/VxUrl.h>
#include <CoreLib/VxPtopUrl.h>

#include <algorithm>

const char* Invite::INVITE_BEGIN = "!Invite!";
const char* Invite::INVITE_HDR_MSG = "!Message!";
const char* Invite::INVITE_HDR_NET_SETTING = "!NetworkSettings!";
const char* Invite::INVITE_END = "!InviteEnd!";
const char* Invite::PTOP_URL_PREFIX = "ptop://";
const char Invite::SUFFIX_CHAR_PERSON_DIRECT = 'D'; // D
const char Invite::SUFFIX_CHAR_PERSON_RELAYED = 'P'; // P
const char Invite::SUFFIX_CHAR_GROUP = 'G'; // G
const char Invite::SUFFIX_CHAR_CHAT_ROOM = 'C'; // C
const char Invite::SUFFIX_CHAR_RANDOM_CONNECT = 'R'; // R
const char Invite::SUFFIX_CHAR_NETWORK_HOST = 'N'; // N
const char Invite::SUFFIX_CHAR_CONNECT_TEST = 'T'; // T
const char Invite::SUFFIX_CHAR_UNKNOWN = 'U'; // 'U'

const char* Invite::INVITE_PRIVATE_KEY = "NetworkKey-Private";

//============================================================================
void Invite::clearInvite( void )
{
    m_InviteText.clear();

    m_PersonUrl.clear();
    m_GroupUrl.clear();
    m_ChatRoomUrl.clear();
    m_RandomConnectUrl.clear();

    m_NetSettingGroupUrl.clear();
    m_NetSettingChatRoomUrl.clear();
    m_NetSettingRandomConnectUrl.clear();
    m_NetSettingNetworkHostUrl.clear();
    m_NetSettingConnectTestUrl.clear();
}

//============================================================================
bool Invite::setInviteText( std::string inviteText )
{
    clearInvite();
    m_InviteText = inviteText;
    return parseInviteText();
}

//============================================================================
bool Invite::setInviteUrl( EHostType hostType, std::string& url, bool isNetworkUrl )
{
    bool result = false;
    if( url.empty() )
    {

    }

    switch( hostType )
    {
    case eHostTypeGroup:
        m_GroupUrl = url;
        VxPtopUrl::setUrlHostType( url, eHostTypeGroup );
        result = !url.empty();
        if( result )
        break;

    case eHostTypeChatRoom:
        m_ChatRoomUrl = url;
        VxPtopUrl::setUrlHostType( m_ChatRoomUrl, eHostTypeChatRoom );
        result = !m_ChatRoomUrl.empty();
        break;

    case eHostTypeRandomConnect:
        m_RandomConnectUrl = url;
        VxPtopUrl::setUrlHostType( m_RandomConnectUrl, eHostTypeRandomConnect );
        result = !m_RandomConnectUrl.empty();
        break;

    case eHostTypePeerUser:
        m_PersonUrl = url;
        VxPtopUrl::setUrlHostType( m_PersonUrl, hostType );
        result = !m_PersonUrl.empty();
        break;

    default:
        break;
    }

    return result;
}

//============================================================================
std::string& Invite::getInviteUrl( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeGroup:
        return m_GroupUrl;

    case eHostTypeChatRoom:
        return m_ChatRoomUrl;

    case eHostTypeRandomConnect:
        return m_RandomConnectUrl;

    case eHostTypePeerUser:
        return m_PersonUrl;

    default:
        return m_EmptyString;
    }
}

//============================================================================
std::string& Invite::getNetSettingUrl( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypeNetwork:
        return m_NetSettingNetworkHostUrl;

    case eHostTypeConnectTest:
        return m_NetSettingConnectTestUrl;

    case eHostTypeGroup:
        return m_NetSettingGroupUrl;

    case eHostTypeChatRoom:
        return m_NetSettingChatRoomUrl;

    case eHostTypeRandomConnect:
        return m_NetSettingRandomConnectUrl;

    default:
        return m_EmptyString;
    }
}

//============================================================================
char Invite::getHostTypeSuffix( EHostType hostType )
{
    switch( hostType )
    {
    case eHostTypePeerUser:
        return SUFFIX_CHAR_PERSON_DIRECT;

    case eHostTypeNetwork:
        return SUFFIX_CHAR_NETWORK_HOST;

    case eHostTypeConnectTest:
        return SUFFIX_CHAR_CONNECT_TEST;

    case eHostTypeGroup:
        return SUFFIX_CHAR_GROUP;

    case eHostTypeChatRoom:
        return SUFFIX_CHAR_CHAT_ROOM;

    case eHostTypeRandomConnect:
        return SUFFIX_CHAR_RANDOM_CONNECT;

    default:
        return SUFFIX_CHAR_UNKNOWN;
    }
}

//============================================================================
EHostType Invite::getHostTypeFromSuffix( const char suffix )
{
    switch( suffix )
    {

    case SUFFIX_CHAR_PERSON_DIRECT:
        return eHostTypePeerUser;

    case SUFFIX_CHAR_NETWORK_HOST:
        return eHostTypeNetwork;

    case SUFFIX_CHAR_CONNECT_TEST:
        return eHostTypeConnectTest;

    case SUFFIX_CHAR_GROUP:
        return eHostTypeGroup;

    case SUFFIX_CHAR_CHAT_ROOM:
        return eHostTypeChatRoom;

    case SUFFIX_CHAR_RANDOM_CONNECT:
        return eHostTypeRandomConnect;

    default:
        return eHostTypeUnknown;
    }
}

//============================================================================
bool Invite::getInviteUrl( EHostType hostType, VxPtopUrl& retUrl )
{
    retUrl.clear();
    std::string url = getInviteUrl( hostType );
    if( !url.empty() )
    {
        retUrl.setUrl( url );
        retUrl.setUrlHostType( hostType );
    }

    return retUrl.isValid();
}

//============================================================================
bool Invite::getNetSettingUrl( EHostType hostType, VxPtopUrl& retUrl )
{
    retUrl.clear();
    std::string url = getNetSettingUrl( hostType );
    if( !url.empty() )
    {
        retUrl.setUrl( url );
        retUrl.setUrlHostType( hostType, true );
    }

    return retUrl.isValid( true );
}

//============================================================================
bool Invite::isValidHostTypeSuffix( const char suffix )
{
    return SUFFIX_CHAR_PERSON_RELAYED == suffix ||
        SUFFIX_CHAR_PERSON_DIRECT == suffix ||
        SUFFIX_CHAR_NETWORK_HOST == suffix ||
        SUFFIX_CHAR_CONNECT_TEST == suffix ||
        SUFFIX_CHAR_GROUP == suffix ||
        SUFFIX_CHAR_CHAT_ROOM == suffix ||
        SUFFIX_CHAR_RANDOM_CONNECT == suffix;
}

//============================================================================
bool Invite::parseInviteText( void )
{
    bool result = !m_InviteText.empty();
    std::vector<std::string> strList;
    if( result )
    {
        m_InviteText.erase( std::remove( m_InviteText.begin(), m_InviteText.end(), '\r' ), m_InviteText.end() );
        StdStringSplit( m_InviteText, '\n', strList );
        result &= !strList.empty();
    }

    if( result )
    {
        bool foundStart = false;
        bool foundMsg = false;
        bool foundEnd = false;
        bool foundUrl = false;
        bool isNetworkUrl = false;
        bool isPrivateNetworkKey = false;
        m_UserMsg.clear();
        for( auto& str : strList )
        {
            if( !foundStart )
            {
                if( str == INVITE_BEGIN )
                {
                    foundStart = true;
                }

                continue;
            }
            
            if( !foundEnd )
            {
                if( str == INVITE_END )
                {
                    foundEnd = true;
                    break;
                }
            }

            if( str == INVITE_HDR_NET_SETTING )
            {
                isNetworkUrl = true;
                continue;
            }

            if( str == "NetworkKey-Private" )
            {
                isPrivateNetworkKey = true;
                continue;
            }

            if( str == "NetworkKey-Public" )
            {
                isPrivateNetworkKey = false;
                continue;
            }

            if( str == INVITE_HDR_MSG )
            {
                foundMsg = true;
                continue;
            }

            if( foundMsg && !isNetworkUrl )
            {
                m_UserMsg += str;
                continue;
            }

            if( str.length() > 10 && str.rfind( PTOP_URL_PREFIX, 0 ) == 0 )
            {
                // a url that begins with ptop://
                // 
                // it should have a Suffix letter that indicates the host type
                char endChar = str.at( str.length() - 1 );
                EHostType hostType = getHostTypeFromSuffix( endChar );
                if( hostType != eHostTypeUnknown )
                {
                    str.erase( str.begin() + str.length() - 1 );
                    VxUrl url( str.c_str() );
                    if( url.getPort() > 0 )
                    {
                        if( isNetworkUrl )
                        {
                            getNetSettingUrl( hostType ) = str;
                        }
                        else
                        {
                            getInviteUrl( hostType ) = str;
                        }

                        foundUrl = true;
                    }
                }
            }
        }

        result &= foundStart && foundEnd && foundUrl;
    }

    return result;
}

//============================================================================
// assumes vaild node url with online id
std::string Invite::makeInviteUrl( EHostType hostType, std::string& onlineUrl )
{
    std::string onlineUrl2 = onlineUrl;
    VxPtopUrl::setUrlHostType( onlineUrl2, hostType );
    return onlineUrl2;
}

//============================================================================
bool Invite::appendHostTypeSuffix( EHostType hostType, std::string& onlineUrl )
{
    return VxPtopUrl::setUrlHostType( onlineUrl, hostType );
}

//============================================================================
bool Invite::getInviteUrls( std::vector<VxPtopUrl>& hostUrls, std::vector<VxPtopUrl>& networkUrls, std::string& userMsg )
{
    hostUrls.clear();
    networkUrls.clear();
    userMsg = m_UserMsg;

    VxPtopUrl ptopUrl;
    if( getInviteUrl( eHostTypePeerUser, ptopUrl ) ) { hostUrls.emplace_back( ptopUrl ); }
    if( getInviteUrl( eHostTypeChatRoom, ptopUrl ) ) { hostUrls.emplace_back( ptopUrl ); }
    if( getInviteUrl( eHostTypeGroup, ptopUrl ) ) { hostUrls.emplace_back( ptopUrl ); }
    if( getInviteUrl( eHostTypeRandomConnect, ptopUrl ) ) { hostUrls.emplace_back( ptopUrl ); }

    if( getNetSettingUrl( eHostTypeNetwork, ptopUrl ) ) { networkUrls.emplace_back( ptopUrl ); }
    if( getNetSettingUrl( eHostTypeConnectTest, ptopUrl ) ) { networkUrls.emplace_back( ptopUrl ); }
    if( getNetSettingUrl( eHostTypeChatRoom, ptopUrl ) ) { networkUrls.emplace_back( ptopUrl ); }
    if( getNetSettingUrl( eHostTypeGroup, ptopUrl ) ) { networkUrls.emplace_back( ptopUrl ); }
    if( getNetSettingUrl( eHostTypeRandomConnect, ptopUrl ) ) { networkUrls.emplace_back( ptopUrl ); }

    return !hostUrls.empty() || !networkUrls.empty();
}

//============================================================================
bool Invite::isInviteTextValid( std::string inviteText )
{
    return textContains( inviteText, INVITE_BEGIN ) && textContains( inviteText, INVITE_END );
}

//============================================================================
bool Invite::textContains( std::string& inviteText, std::string searchText )
{
    return !searchText.empty() && ( inviteText.find( searchText ) != std::string::npos );
}

//============================================================================
bool Invite::inviteHasPrivateNetworkKey( std::string inviteText )
{
    return textContains( inviteText, INVITE_PRIVATE_KEY );
}

