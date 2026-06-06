//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetPlaySession.h"

//============================================================================
AssetPlaySession::AssetPlaySession()
    : AssetBaseInfo()
{ 
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType )
    : AssetBaseInfo(assetType)
{ 
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( const AssetPlaySession& rhs )
: AssetBaseInfo( rhs )
, m_SessionId( rhs.m_SessionId )
{   
}

//============================================================================
AssetPlaySession::AssetPlaySession( const AssetBaseInfo& rhs )
: AssetBaseInfo( rhs )
{   
    m_SessionId.initializeWithNewVxGUID();
    setPlayPosition( 0 );
}

//============================================================================
AssetPlaySession::AssetPlaySession( const AssetBaseInfo& rhs, VxGUID& sessionId, int pos0to100000 )
: AssetBaseInfo( rhs )
, m_SessionId( sessionId )
{   
    setPlayPosition( pos0to100000 );
}

//============================================================================
AssetPlaySession::AssetPlaySession( const VxFileInfoBase& rhs )
: AssetBaseInfo( rhs )
{   
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( FileInfo& rhs )
    : AssetBaseInfo( rhs )
{
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType, VxGUID& creatorId, int64_t modifiedTime )
: AssetBaseInfo( assetType, creatorId, modifiedTime )
{
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType,  VxGUID& creatorId, VxGUID& assetId, int64_t modifiedTime )
: AssetBaseInfo( assetType, creatorId, assetId, modifiedTime )
{
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType, std::string fileName, std::string fileNameAndPath )
: AssetBaseInfo( assetType, fileName, fileNameAndPath )
{ 
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType, std::string fileName, std::string fileNameAndPath, VxGUID& assetId )
    : AssetBaseInfo( assetType, fileName, fileNameAndPath, assetId )
{
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen )
: AssetBaseInfo( assetType, fileName, fileNameAndPath, fileLen )
{
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession::AssetPlaySession( EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen, VxGUID& assetId )
    : AssetBaseInfo( assetType, fileName, fileNameAndPath, fileLen, assetId )
{
    m_SessionId.initializeWithNewVxGUID();
}

//============================================================================
AssetPlaySession& AssetPlaySession::operator=( const AssetPlaySession& rhs )
{
    if( &rhs != this )
    {
        *((AssetBaseInfo*)this) = rhs;
        m_SessionId = rhs.m_SessionId;
    }

    return *this;
}