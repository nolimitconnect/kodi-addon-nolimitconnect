//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "BaseInfo.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/FileInfo.h>

#include <PktLib/VxCommon.h>
#include <CoreLib/PktBlobEntry.h>

#include <CoreLib/VxDebug.h>

#include <GuiInterface/IToGui.h>

//============================================================================
BaseInfo::BaseInfo( VxGUID& onlineId, int64_t modifiedTime )
    : m_OnlineId( onlineId )
    , m_InfoModifiedTime( modifiedTime )
{
    assureHasCreatorId();
    assureValidTimes();
}

//============================================================================
BaseInfo::BaseInfo( VxGUID& creatorId, VxGUID& assetId, int64_t modifiedTime )
    : m_OnlineId( creatorId )
    , m_ThumbId( assetId )
    , m_InfoModifiedTime( modifiedTime )
{
    assureHasCreatorId();
    assureValidTimes();
}

//============================================================================
BaseInfo::BaseInfo( const BaseInfo& rhs )
    : m_OnlineId( rhs.m_OnlineId )
    , m_ThumbId( rhs.m_ThumbId )
    , m_InfoModifiedTime( rhs.m_InfoModifiedTime )
{
    assureHasCreatorId();
    assureValidTimes();
}

//============================================================================
BaseInfo::BaseInfo( const FileInfo& rhs )
    : m_OnlineId( rhs.m_OnlineId )
    , m_ThumbId( rhs.m_ThumbId )
    , m_InfoModifiedTime( rhs.m_FileTime )
{
    assureHasCreatorId();
    assureValidTimes();
}

//============================================================================
BaseInfo& BaseInfo::operator=( const BaseInfo& rhs )
{
    if( this != &rhs )
    {
        m_OnlineId = rhs.m_OnlineId;
        m_ThumbId = rhs.m_ThumbId;
        m_InfoModifiedTime = rhs.m_InfoModifiedTime;
    }

    return *this;
}

//============================================================================
bool BaseInfo::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_OnlineId );
    result &= blob.setValue( m_ThumbId );
    result &= blob.setValue( m_InfoModifiedTime );
    return result;
}

//============================================================================
bool BaseInfo::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_OnlineId );
    result &= blob.getValue( m_ThumbId );
    result &= blob.getValue( m_InfoModifiedTime );
    return result;
}

//============================================================================
void BaseInfo::fillBaseInfo( VxNetIdent* netIdent, EHostType hostType )
{
    vx_assert( netIdent );

    m_OnlineId = netIdent->getMyOnlineId();
    m_ThumbId = netIdent->getThumbId( hostType );
    m_InfoModifiedTime = GetTimeStampMs();
}

//============================================================================
void BaseInfo::assureHasCreatorId( void )
{
    if( !m_OnlineId.isValid() )
    {
        m_OnlineId = GetPtoPEngine().getMyOnlineId();
    }
}

//============================================================================
void BaseInfo::printValues( uint32_t logMsgType ) const
{
    LogMsg( logMsgType, "*Begin BaseInfo" );

    LogMsg( logMsgType, "m_OnlineId=(%s)", m_OnlineId.toOnlineIdString().c_str() );
    LogMsg( logMsgType, "m_ThumbId=(%s)", m_ThumbId.toOnlineIdString().c_str() );
    LogMsg( logMsgType, "m_InfoModifiedTime=(%lld)", m_InfoModifiedTime );

    LogMsg( logMsgType, "*End BaseInfo" );
}

//============================================================================
void BaseInfo::assureValidTimes( void )
{
    if( !m_InfoModifiedTime )
    {
        m_InfoModifiedTime = GetTimeStampMs();
    }
}