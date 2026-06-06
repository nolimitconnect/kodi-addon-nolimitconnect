#pragma once
//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPktHdr.h"
#include "VxCommon.h"
#include <CoreLib/HostedId.h>

#define PKT_ANN_ANNOUNCE_VERSION 1
#define P2P_ENGINE_VERSION 1

#pragma pack(push)
#pragma pack(1)

//=== request FLAGS ===//
#define FLAG_PKT_ANN_REQ_REPLY					0x01
#define FLAG_PKT_ANN_REQ_REV_CONNECT			0x02
#define FLAG_PKT_ANN_REQ_STUN					0x04
#define FLAG_PKT_ANN_TEMP_CONNECT				0x08

// +  40 bytes VxPktHdr
// + 432 bytes VxNetIdent
// = 472 bytes
// 
// +  1 m_u8TimeToLive
// +  1 m_u8RequestFlags
// +  1 m_HostAction
// +  1 m_HostType
// + 16 m_HostOnlineId
// +  4 m_u32AppAliveTimeSec
// + 16 m_AnnRes 1-4
// = 40 bytes
// 
// 40 + 472 = 512 bytes
class PktAnnBase : public VxPktHdr, public VxNetIdent
{
public:
    PktAnnBase() = default;
    PktAnnBase( const PktAnnBase& rhs );
    PktAnnBase&		            operator = ( const PktAnnBase& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    void						setIsPktAnnReplyRequested( bool bReqReply );
    bool						getIsPktAnnReplyRequested( void );
    void						setIsPktAnnRevConnectRequested( bool bReqConnect );
    bool						getIsPktAnnRevConnectRequested( void );
    void						setIsPktAnnStunRequested( bool bReqStun );
    bool						getIsPktAnnStunRequested( void );
    void						setIsPktAnnTempConnection( bool isTemp );
    bool						getIsPktAnnTempConnection( void );

    void						setTTL( uint8_t timeToLive )                { m_TimeToLive = timeToLive; }
    uint8_t						getTTL( void )                              { return m_TimeToLive; }

    void                        setHostType( EHostType hostType )           { m_HostType = (uint8_t)hostType; }
    EHostType					getHostType( void )                         { return (EHostType)m_HostType; }

    void                        setHostOnlineId( VxGUID& onlineId )         { m_HostOnlineId = onlineId; }
    VxGUID&					    getHostOnlineId( void )                     { return m_HostOnlineId; }

    void						setAppAliveTimeSec( uint32_t aliveTime )    { m_u32AppAliveTimeSec = aliveTime; }
    uint32_t					getAppAliveTimeSec( void )                  { return m_u32AppAliveTimeSec; }

    void                        setHostId( HostedId& hostedId );
    void                        setHostInfo( VxGUID& hostOnlineId, EHostType hostType );
    void                        clearTempValues( void );

    //=== vars ===//
    uint8_t						m_TimeToLive{ 0 };
    uint8_t						m_u8RequestFlags{ 0 };	// request flags
    uint8_t						m_HostAction{ 0 };
    uint8_t				        m_HostType{ 0 };

    VxGUID				        m_HostOnlineId;		

    uint32_t				    m_u32AppAliveTimeSec{ 0 };
    uint32_t				    m_AnnRes1{ 0 };
    uint32_t					m_AnnRes2{ 0 };
    uint32_t					m_AnnRes3{ 0 };
    uint32_t				    m_AnnRes4{ 0 };
};

class PktAnnounce : public PktAnnBase
{
public:
	PktAnnounce();
    PktAnnounce( const PktAnnounce& rhs );
    PktAnnounce&				operator = ( const PktAnnounce& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	bool						isValidPktAnn( void );

    VxNetIdent*                 getVxNetIdent( void ) { return this; } // access from this to avoid virtual table cast conversion issues with derived classes

	bool						isOnlineStatusExpired( void );
	bool						isNearbyStatusExpired( void );
	void						updateNearbyPermissions( void );

	PktAnnounce*				makeAnnCopy( void );
	PktAnnounce*				makeAnnReverseCopy( void );
	void						DebugDump( void );
};

#pragma pack(pop)


