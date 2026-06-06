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

#include <NetLib/VxSktBase.h>

class P2PEngine;

class VxSktLoopback : public VxSktBase
{
public:
    VxSktLoopback() = delete;	
    VxSktLoopback( P2PEngine& engine );	
	virtual ~VxSktLoopback() = default;

    virtual bool				isConnected( void ) override                { return true; }

    virtual int32_t				txPacketWithDestId(	VxPktHdr* pktHdr, bool sktMgrLocked = false ) override;	

    //! return true if transmit encryption key is set
    virtual bool				isTxEncryptionKeySet( void ) override       { return true; }
    //! return true if receive encryption key is set
    virtual bool				isRxEncryptionKeySet( void ) override       { return true; }

    void						lockPktList( void )                         { m_PktListMutex.lock(); }
    void						unlockPktList( void )                       { m_PktListMutex.unlock(); }

    void                        enableSktLoopbackThread( bool enable );
    void				        executeSktLoopbackRxThreadFunc( void );
    void				        checkForMorePacketsAfterThreadExit( void );

protected:
    P2PEngine&                  m_Engine;
    std::vector<VxPktHdr*>      m_PktList;
    VxMutex                     m_PktListMutex;
    VxThread                    m_SktLoopbackThread;
};
