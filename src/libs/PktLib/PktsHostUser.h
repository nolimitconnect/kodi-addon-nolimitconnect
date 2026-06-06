#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/GroupieId.h>
#include "PktTypes.h"
#include <CoreLib/PktBlobEntry.h>

#include <GuiInterface/IDefs.h>

#pragma pack(push)
#pragma pack(1)
class PktHostUserInfoReq : public VxPktHdr
{
public:
    PktHostUserInfoReq();

    void						setSessionId( VxGUID& guid )        { m_SessionId = guid; }
    VxGUID&                     getSessionId( void )                { return m_SessionId; }

    void						setGroupieId( GroupieId& groupie )  { m_GroupieId = groupie; }
    GroupieId&                  getGroupieId( void )                { return m_GroupieId; }

    void                        setCommError( enum ECommErr commError )  { m_CommError = ( uint8_t )commError; }
    ECommErr                    getCommError( void ) const          { return ( ECommErr )m_CommError; }

private:
    //=== vars ===//
    // VxPktHdr 40 bytes 
    VxGUID                      m_SessionId;        // 16 bytes
    GroupieId                   m_GroupieId;        // 33 bytes
    uint8_t					    m_CommError{ 0 };   // 1 byte
    // 90 bytes to here
    uint16_t                    m_Res1{ 0 };        // fill to 16 byte boundry   
    uint32_t                    m_Res2{ 0 };        // fill to 16 byte boundry   
};

class PktHostUserInfoReply : public VxPktHdr
{
public:
    PktHostUserInfoReply();

    void                        calcPktLen( void );

    void						setSessionId( VxGUID& guid )        { m_SessionId = guid; }
    VxGUID&                     getSessionId( void )                { return m_SessionId; }

    void						setGroupieId( GroupieId& groupie )  { m_GroupieId = groupie; }
    GroupieId&                  getGroupieId( void )                { return m_GroupieId; }
    EHostType                   getHostType( void )                 { return m_GroupieId.getHostType(); }

    void                        setCommError( enum ECommErr commError ) { m_CommError = ( uint8_t )commError; }
    ECommErr                    getCommError( void ) const          { return ( ECommErr )m_CommError; }

    PktBlobEntry&               getBlobEntry( void )                { return m_BlobEntry; }

private:
    //=== vars ===//
    // VxPktHdr 40 bytes 
    VxGUID                      m_SessionId;        // 16 bytes
    GroupieId                   m_GroupieId;        // 33 bytes
    uint8_t					    m_CommError{ 0 };   // 1 byte
    // 90 bytes to here
    uint16_t					m_Res1{ 0 };	    // 2 bytes
    uint32_t                    m_Res2{ 0 };        // fill to 16 byte boundry   
    // 96 bytes to here
    PktBlobEntry                m_BlobEntry;	    // size 14352
};


class PktHostUserStatusReq : public VxPktHdr
{
public:
    PktHostUserStatusReq();

    void						setSessionId( VxGUID& guid )            { m_SessionId = guid; }
    VxGUID&                     getSessionId( void )                    { return m_SessionId; }

    void						setGroupieId( GroupieId& groupie )      { m_GroupieId = groupie; }
    GroupieId&                  getGroupieId( void )                    { return m_GroupieId; }

    void                        setCommError( enum ECommErr commError ) { m_CommError = (uint8_t)commError; }
    ECommErr                    getCommError( void ) const              { return (ECommErr)m_CommError; }

private:
    //=== vars ===//
    // VxPktHdr 40 bytes 
    VxGUID                      m_SessionId;        // 16 bytes
    GroupieId                   m_GroupieId;        // 33 bytes
    uint8_t					    m_CommError{ 0 };   // 1 byte
     // 90 bytes to here
    uint8_t                     m_Status{ 0 };      // fill to 16 byte boundry
    uint8_t                     m_Res1{ 0 };        // fill to 16 byte boundry
    uint32_t                    m_Res2{ 0 };        // fill to 16 byte boundry
};

class PktHostUserStatusReply : public VxPktHdr
{
public:
    PktHostUserStatusReply();

    void                        calcPktLen( void );

    void						setSessionId( VxGUID& guid ) { m_SessionId = guid; }
    VxGUID&                     getSessionId( void ) { return m_SessionId; }

    void                        setHostType( EHostType hostType ) { m_HostType = (uint8_t)hostType; }
    EHostType                   getHostType( void ) const { return (EHostType)m_HostType; }

    void						setHostOnlineId( VxGUID& guid ) { m_HostOnlineId = guid; }
    VxGUID&                     getHostOnlineId( void ) { return m_HostOnlineId; }

    void                        setCommError( enum ECommErr commError ) { m_CommError = (uint8_t)commError; }
    ECommErr                    getCommError( void ) const { return (ECommErr)m_CommError; }

    bool                        setHostUserUrlAndTitleAndDescription( std::string& groupieUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );
    bool                        getHostUserUrlAndTitleAndDescription( std::string& groupieUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );

    PktBlobEntry&               getBlobEntry( void ) { return m_BlobEntry; }

private:
    //=== vars ===//
    // VxPktHdr 40 bytes 
    VxGUID                      m_SessionId;            // 16 bytes
    uint8_t					    m_HostType{ 0 };        // 1 byte
    uint8_t					    m_CommError{ 0 };       // 1 byte
    uint16_t					m_Res1{ 0 };	        // 2 bytes
    VxGUID                      m_HostOnlineId;  // 16 bytes
    // 60 bytes to here
    PktBlobEntry                m_BlobEntry;	//size 14352
};

class PktHostUserListReq : public VxPktHdr
{
public:
    PktHostUserListReq();

    void                        setHostId( HostedId& hostId );
    HostedId                    getHostId( void );

    void                        setHostType( enum EHostType hostType )  { m_HostType = ( uint8_t )hostType; }
    EHostType                   getHostType( void ) const               { return ( EHostType )m_HostType; }

    void						setHostOnlineId( VxGUID& guid )         { m_HostOnlineId = guid; }
    VxGUID&                     getHostOnlineId( void )                 { return m_HostOnlineId; }

    void						setSearchSessionId( VxGUID& guid )      { m_SearchSessionId = guid; }
    VxGUID&                     getSearchSessionId( void )              { return m_SearchSessionId; }

    void						setSpecificOnlineId( VxGUID& guid )     { m_SpecificOnlineId = guid; }
    VxGUID&                     getSpecificOnlineId( void )             { return m_SpecificOnlineId; }

protected:
    uint8_t					    m_HostType{ 0 };
    uint8_t					    m_Res1{ 0 };
    uint16_t                    m_Res2{ 0 };
    uint32_t                    m_Res3{ 0 };
    VxGUID                      m_SearchSessionId;
    VxGUID                      m_HostOnlineId;  // 16 bytes
    VxGUID                      m_SpecificOnlineId;	
};

class PktHostUserListReply : public VxPktHdr
{
public:
    PktHostUserListReply();

    void                        setHostType( enum EHostType hostType )       { m_HostType = ( uint8_t )hostType; }
    EHostType                   getHostType( void ) const               { return ( EHostType )m_HostType; }

    void						setHostOnlineId( VxGUID& guid )         { m_HostOnlineId = guid; }
    VxGUID&                     getHostOnlineId( void )                 { return m_HostOnlineId; }

    void						setSearchSessionId( VxGUID& guid )      { m_SearchSessionId = guid; }
    VxGUID&                     getSearchSessionId( void )              { return m_SearchSessionId; }

    void                        setCommError( enum ECommErr commError )      { m_CommError = ( uint8_t )commError; }
    ECommErr                    getCommError( void ) const              { return ( ECommErr )m_CommError; }

    bool                        addHostUserInfo( std::string& groupieUrl, std::string& groupieTitle, std::string& groupieDesc, int64_t& lastModifiedTime );

    void						setHostUserCountThisPkt( uint16_t inviteCnt ) { m_HostUserThisPktCount = inviteCnt; }
    uint16_t&                   getHostUserCountThisPkt( void )           { return m_HostUserThisPktCount; }
    void                        incrementHostUserCount( void )            { m_HostUserThisPktCount++; }

    void						setMoreHostUsersExist( bool moreExist)    { m_MoreHostUsersExist = moreExist; }
    bool                        getMoreHostUsersExist( void )             { return m_MoreHostUsersExist; }

    void						setNextSearchOnlineId( VxGUID& guid )   { m_NextSearchOnlineId = guid; }
    VxGUID&                     getNextSearchOnlineId( void )           { return m_NextSearchOnlineId; }

    PktBlobEntry&               getBlobEntry( void )                    { return m_BlobEntry; }

    void                        calcPktLen( void );

protected:
    uint8_t					    m_HostType{ 0 };
    uint8_t					    m_MoreHostUsersExist{ 0 };
    uint8_t                     m_CommError{ 0 };
    uint8_t                     m_Res1{ 0 };
    uint16_t                    m_HostUserThisPktCount{ 0 };
    uint16_t                    m_Res2{ 0 };
    VxGUID                      m_SearchSessionId;
    VxGUID                      m_HostOnlineId;
    VxGUID                      m_NextSearchOnlineId;
    PktBlobEntry                m_BlobEntry;
};


class PktHostUserListMoreReq : public VxPktHdr
{
public:
    PktHostUserListMoreReq();

    void                        setHostType( enum EHostType hostType )   { m_HostType = ( uint8_t )hostType; }
    EHostType                   getHostType( void ) const           { return ( EHostType )m_HostType; }

    void						setHostOnlineId( VxGUID& guid )     { m_HostOnlineId = guid; }
    VxGUID&                     getHostOnlineId( void )             { return m_HostOnlineId; }

    void						setSearchSessionId( VxGUID& guid )  { m_SearchSessionId = guid; }
    VxGUID&                     getSearchSessionId( void )          { return m_SearchSessionId; }

    void						setNextSearchOnlineId( VxGUID& guid ) { m_NextSearchOnlineId = guid; }
    VxGUID&                     getNextSearchOnlineId( void )        { return m_NextSearchOnlineId; }

protected:
    uint8_t					    m_HostType{ 0 };
    uint8_t					    m_Res1{ 0 };
    uint16_t                    m_Res2{ 0 };
    uint32_t                    m_Res3{ 0 };
    VxGUID                      m_SearchSessionId;
    VxGUID                      m_HostOnlineId;
    VxGUID                      m_NextSearchOnlineId;
};

class PktHostUserListMoreReply : public VxPktHdr
{
public:
    PktHostUserListMoreReply();

    void                        setHostType( enum EHostType hostType )   { m_HostType = ( uint8_t )hostType; }
    EHostType                   getHostType( void ) const           { return ( EHostType )m_HostType; }

    void						setHostOnlineId( VxGUID& guid )     { m_HostOnlineId = guid; }
    VxGUID&                     getHostOnlineId( void )             { return m_HostOnlineId; }

    void						setSearchSessionId( VxGUID& guid )  { m_SearchSessionId = guid; }
    VxGUID&                     getSearchSessionId( void )          { return m_SearchSessionId; }

    void                        setCommError( enum ECommErr commError )  { m_CommError = ( uint8_t )commError; }
    ECommErr                    getCommError( void ) const          { return ( ECommErr )m_CommError; }

    void						setHostUserCountThisPkt( uint16_t inviteCnt ) { m_HostUserThisPktCount = inviteCnt; }
    uint16_t&                   getHostUserCountThisPkt( void )       { return m_HostUserThisPktCount; }
    void                        incrementHostUserCount( void )        { m_HostUserThisPktCount++; }

    void						setMoreHostUsersExist( bool moreExist ) { m_MoreHostUsersExist = moreExist; }
    bool                        getMoreHostUsersExist( void )         { return m_MoreHostUsersExist; }

    void						setNextSearchOnlineId( VxGUID& guid ) { m_NextSearchOnlineId = guid; }
    VxGUID&                     getNextSearchOnlineId( void )       { return m_NextSearchOnlineId; }

    PktBlobEntry&               getBlobEntry( void )                { return m_BlobEntry; }

    void                        calcPktLen( void );

protected:
    uint8_t					    m_HostType{ 0 };
    uint8_t					    m_MoreHostUsersExist{ 0 };
    uint8_t                     m_CommError{ 0 };
    uint8_t                     m_Res1{ 0 };
    uint16_t                    m_HostUserThisPktCount{ 0 };
    uint16_t                    m_Res2{ 0 };
    VxGUID                      m_SearchSessionId;
    VxGUID                      m_HostOnlineId;
    VxGUID                      m_NextSearchOnlineId;
    PktBlobEntry                m_BlobEntry;
};

#pragma pack(pop)


