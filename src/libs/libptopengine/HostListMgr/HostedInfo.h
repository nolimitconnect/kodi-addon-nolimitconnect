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

#include <CoreLib/GroupieId.h>

#include <string>

class PktHostInviteAnnounceReq;

class HostedInfo
{
public:
	HostedInfo() = default;
	HostedInfo( const HostedInfo& rhs );
    HostedInfo( EHostType hostType, VxGUID& onlineId, std::string& hostUrl, VxGUID& thumbId );
    HostedInfo( HostedId& adminId, std::string& hostUrl );
    virtual ~HostedInfo() = default;

	HostedInfo&				    operator=( const HostedInfo& rhs ); 

    bool                        isHostInviteValid( void );

    virtual void				setAdminId( HostedId& hostedId )                    { m_AdminId = hostedId; }
    HostedId&                   getAdminId( void )                                  { return m_AdminId; }

    virtual void                setAdminOnlineId( VxGUID& onlineId )                { return m_AdminId.setHostOnlineId( onlineId ); }
    virtual VxGUID&             getAdminOnlineId( void )                            { return m_AdminId.getHostOnlineId(); }

    virtual void			    setHostType( enum EHostType hostType )              { m_AdminId.setHostType( hostType ); }
    virtual EHostType	        getHostType( void )                                 { return m_AdminId.getHostType(); }

    EPluginType                 getHostPluginType( void );
    EPluginType                 getClientPluginType( void );

    virtual void				setThumbId( VxGUID& thumbId )                       { m_ThumbId = thumbId; }
    virtual VxGUID&             getThumbId( void )                                  { return m_ThumbId; }

    virtual void                setIsFavorite( bool isFavorite )                    { m_IsFavorite = isFavorite; }
    virtual bool                getIsFavorite( void )                               { return m_IsFavorite; }

    virtual void			    setConnectedTimestamp( int64_t timestampMs )        { m_ConnectedTimestampMs = timestampMs; }
    virtual int64_t             getConnectedTimestamp( void )                       { return m_ConnectedTimestampMs; }
    virtual void			    setJoinedTimestamp( int64_t timestampMs )           { m_JoinedTimestampMs = timestampMs; }
    virtual int64_t             getJoinedTimestamp( void )                          { return m_JoinedTimestampMs; }
    virtual void			    setHostInfoTimestamp( int64_t timestampMs )         { m_HostInfoTimestampMs = timestampMs; }
    virtual int64_t             getHostInfoTimestamp( void )                        { return m_HostInfoTimestampMs; }

    virtual void			    setHostInviteUrl( std::string hostUrl )             { if( hostUrl.empty() ) return; m_HostInviteUrl = hostUrl; }
    virtual std::string&        getHostInviteUrl( void )                            { return m_HostInviteUrl; }

    virtual void                setHostTitle( std::string hostTitle )               { if( hostTitle.empty() ) return; m_HostTitle = hostTitle; }
    virtual std::string&        getHostTitle( void )                                { return m_HostTitle; }

    virtual void                setHostDescription( std::string hostDesc )          { if( hostDesc.empty() ) return; m_HostDesc = hostDesc; }
    virtual std::string&        getHostDescription( void )                          { return m_HostDesc; }

    bool                        shouldSaveToDb( void );
    bool                        isValidForGui( void );

    bool                        fillFromHostInvite( PktHostInviteAnnounceReq* hostAnn );
    int                         getSearchBlobSpaceRequirement( void );
    bool                        fillSearchBlob( PktBlobEntry& blobEntry );
    bool                        extractFromSearchBlob( PktBlobEntry& blobEntry );

protected:
	//=== vars ===//
    HostedId                    m_AdminId;
    int64_t                     m_ConnectedTimestampMs{ 0 };
    int64_t                     m_JoinedTimestampMs{ 0 };
    int64_t                     m_HostInfoTimestampMs{ 0 };
    bool                        m_IsFavorite{ false };
    std::string                 m_HostInviteUrl;
    std::string                 m_HostTitle;
    std::string                 m_HostDesc;
    VxGUID                      m_ThumbId;
};
