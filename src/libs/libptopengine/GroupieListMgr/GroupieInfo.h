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

class PktGroupieAnnounceReq;
class GroupieId;

class GroupieInfo
{
public:
	GroupieInfo() = default;
	GroupieInfo( const GroupieInfo& rhs );
    GroupieInfo( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, std::string& groupieUrl );
    GroupieInfo( GroupieId& groupieId, std::string& groupieUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t timeModified );
    virtual ~GroupieInfo() = default;

	GroupieInfo&				operator=( const GroupieInfo& rhs ); 

    bool                        isGroupieValid( void );
    bool                        isIdMatch( GroupieId& groupieId );
    bool                        isSearchTextMatch( std::string& searchText );

    bool                        setGroupieUrlAndTitleAndDescription( std::string& groupieUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );
    bool                        getGroupieUrlAndTitleAndDescription( std::string& groupieUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );

    void				        setGroupieId( GroupieId& onlineId )                 { m_GroupieId = onlineId; }
    GroupieId&                  getGroupieId( void )                                { return m_GroupieId; }

    void				        setUserOnlineId( VxGUID& onlineId )                 { m_GroupieId.setUserOnlineId( onlineId ); }
    VxGUID&                     getUserOnlineId( void )                             { return m_GroupieId.getUserOnlineId(); }

    void				        setHostOnlineId( VxGUID& onlineId )                 { m_GroupieId.setHostOnlineId( onlineId ); }
    VxGUID&                     getHostOnlineId( void )                             { return m_GroupieId.getHostOnlineId(); }

    void			            setHostType( enum EHostType hostType )              { m_GroupieId.setHostType( hostType ); }
    EHostType	                getHostType( void )                                 { return m_GroupieId.getHostType(); }

    EPluginType	                getHostPluginType( void );
    EPluginType	                getClientPluginType( void );

    virtual void                setIsFavorite( bool isFavorite )                    { m_IsFavorite = isFavorite; }
    virtual bool                getIsFavorite( void )                               { return m_IsFavorite; }

    virtual void			    setConnectedTimestamp( int64_t timestampMs )        { m_ConnectedTimestampMs = timestampMs; }
    virtual int64_t             getConnectedTimestamp( void )                       { return m_ConnectedTimestampMs; }
    virtual void			    setJoinedTimestamp( int64_t timestampMs )           { m_JoinedTimestampMs = timestampMs; }
    virtual int64_t             getJoinedTimestamp( void )                          { return m_JoinedTimestampMs; }
    virtual void			    setGroupieInfoTimestamp( int64_t timestampMs )      { m_GroupieInfoTimestampMs = timestampMs; }
    virtual int64_t             getGroupieInfoTimestamp( void )                     { return m_GroupieInfoTimestampMs; }

    virtual void			    setGroupieUrl( std::string hostUrl )                { m_GroupieUrl = hostUrl; }
    virtual std::string&        getGroupieUrl( void )                               { return m_GroupieUrl; }

    virtual void                setGroupieTitle( std::string hostTitle )            { m_GroupieTitle = hostTitle; }
    virtual std::string&        getGroupieTitle( void )                             { return m_GroupieTitle; }

    virtual void                setGroupieDescription( std::string hostDesc )       { m_GroupieDesc = hostDesc; }
    virtual std::string&        getGroupieDescription( void )                       { return m_GroupieDesc; }

    bool                        shouldSaveToDb( void );
    bool                        isValidForGui( void );

    bool                        fillFromGroupie( PktGroupieAnnounceReq* hostAnn );
    int                         getSearchBlobSpaceRequirement( void );
    bool                        fillSearchBlob( PktBlobEntry& blobEntry );
    bool                        extractFromSearchBlob( PktBlobEntry& blobEntry );

protected:
	//=== vars ===//
    GroupieId                   m_GroupieId;
    int64_t                     m_ConnectedTimestampMs{ 0 };
    int64_t                     m_JoinedTimestampMs{ 0 };
    int64_t                     m_GroupieInfoTimestampMs{ 0 };
    bool                        m_IsFavorite{ false };
    std::string                 m_GroupieUrl;
    std::string                 m_GroupieTitle;
    std::string                 m_GroupieDesc;
};
