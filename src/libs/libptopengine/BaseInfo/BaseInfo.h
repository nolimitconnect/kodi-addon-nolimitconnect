#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <CoreLib/VxGUID.h>

class FileInfo;
class PktBlobEntry;
class VxNetIdent;

class BaseInfo
{
public:
    BaseInfo() = default;
    BaseInfo( VxGUID& creatorId, int64_t modifiedTime = 0 );
    BaseInfo( VxGUID& creatorId, VxGUID& assetId, int64_t modifiedTime = 0 );
	BaseInfo( const BaseInfo& rhs );
    BaseInfo( const FileInfo& rhs );

    virtual ~BaseInfo() = default;

	BaseInfo&				    operator=( const BaseInfo& rhs ); 

    virtual bool                addToBlob( PktBlobEntry& blob );
    virtual bool                extractFromBlob( PktBlobEntry& blob );

    virtual void				setOnlineId( VxGUID onlineId )                  { m_OnlineId = onlineId; }
    virtual void				setOnlineId( const char* onlineId )             { m_OnlineId.fromVxGUIDHexString( onlineId ); }
    virtual VxGUID&				getOnlineId( void )                             { return m_OnlineId; }

    virtual void				setThumbId( VxGUID& thumbId )                   { m_ThumbId = thumbId; }
    virtual void				setThumbId( const char* thumbId )               { m_ThumbId.fromVxGUIDHexString( thumbId ); }
    virtual VxGUID&				getThumbId( void )                              { return m_ThumbId; }

    virtual void				setInfoModifiedTime( int64_t timestamp )        { m_InfoModifiedTime = timestamp; }
    virtual int64_t			    getInfoModifiedTime( void )                     { return m_InfoModifiedTime; }

    virtual void                fillBaseInfo( VxNetIdent* netIdent, EHostType hostType );

    virtual void                assureHasCreatorId( void );

    virtual void                printValues( uint32_t logMsgType = 1 ) const;

protected:
    virtual void                assureValidTimes( void );

public:
	//=== vars ===//
    VxGUID						m_OnlineId; 
    VxGUID						m_ThumbId; 
    int64_t						m_InfoModifiedTime{ 0 };
};
