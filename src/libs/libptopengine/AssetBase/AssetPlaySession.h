#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetBaseInfo.h"

class AssetPlaySession : public AssetBaseInfo
{
public:
    AssetPlaySession();
    AssetPlaySession( const AssetPlaySession& rhs );
    AssetPlaySession( const AssetBaseInfo& rhs );
    AssetPlaySession( const AssetBaseInfo& rhs, VxGUID& sessionId, int pos0to100000 );
    AssetPlaySession( const VxFileInfoBase& rhs );
    AssetPlaySession( FileInfo& rhs );
    AssetPlaySession( enum EAssetType assetType );
    AssetPlaySession( enum EAssetType assetType, VxGUID& onlineId, int64_t modifiedTime = 0 );
    AssetPlaySession( enum EAssetType assetType, VxGUID& onlineId, VxGUID& assetId, int64_t modifiedTime = 0 );
    AssetPlaySession( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath );
    AssetPlaySession( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, VxGUID& assetId );
    AssetPlaySession( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen );
    AssetPlaySession( enum EAssetType assetType, std::string fileName, std::string fileNameAndPath, uint64_t fileLen, VxGUID& assetId );

    virtual ~AssetPlaySession() = default;

    AssetPlaySession&				operator=( const AssetPlaySession& rhs );

    void                            setSessionId( VxGUID& sessionId ) { m_SessionId = sessionId; }
    VxGUID&                         getSessionId( void ) { return m_SessionId; }

protected:
    VxGUID                          m_SessionId;
};
