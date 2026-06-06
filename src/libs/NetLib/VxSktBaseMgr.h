#pragma once
//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktBase.h"
#include "VxSktDefs.h"
#include "VxSktStatRecord.h"
#include "VxSktThrottle.h"

#include <CoreLib/InetAddress.h>
#include <CoreLib/VxMutex.h>

#include <vector>
#include <memory>

#define SKT_ALIVE_TIMEOUT (2 * 60 * 1000)

class VxGUID;

// implements a manager to manage multiple sockets
// NOTE: applications should use VxServerMgr or ConnnectMgr
class VxSktBaseMgr : public VxSktThrottle
{
public:
	VxSktBaseMgr();
	
	virtual ~VxSktBaseMgr();
	virtual void				setRxCallbackUserData( void* pvData )			{ m_pvRxCallbackUserData = pvData; }
	virtual void *				getRxCallbackUserData( void )					{ return m_pvRxCallbackUserData; }
	virtual void				setTxCallbackUserData( void* pvData )			{ m_pvTxCallbackUserData = pvData; }
	virtual void *				getTxCallbackUserData( void )					{ return m_pvTxCallbackUserData; }

	//virtual void				sktMgrSetLocalIp( InetAddress& oLclIp );
	virtual void				sktMgrStartup( bool ipv6 ) = 0;
	virtual void				sktMgrShutdown( void );

	virtual void				lockSktBaseMgr( void ) { m_SktMgrMutex.lock(); }
	virtual void				unlockSktBaseMgr( void ) { m_SktMgrMutex.unlock(); }

	virtual int					getActiveSktCnt( void );
	virtual int					getToDeleteSktCnt( void );

	//! make a new socket... give derived classes a chance to override
	virtual std::shared_ptr<VxSktBase>			makeNewSkt( void );
	//! make a new socket... give derived classes a chance to override
	virtual std::shared_ptr<VxSktBase>			makeNewAcceptSkt( void );

	//! User must Set Receive Callback
	virtual void				setReceiveCallback( VX_SKT_CALLBACK pfnReceive, void* pvUserData = nullptr );
	virtual void				setSktMgrStatusCallback( VX_SKT_MGR_STATUS_CALLBACK pfnSktMgrStatus, void* pvUserData = nullptr );

	virtual void				handleSktCloseEvent( std::shared_ptr<VxSktBase>& sktBase );
	
	//! Send to all connections.. if crypto is set send encrypted else send raw
	virtual void				sendToAll( char * pvData, int iDataLen, bool sktMgrLocked = false );			// length of data

	//! get number of connected sockets
	virtual int					getConnectedCount( void );
	//! get total number of sockets ( includes not connected sockets )
	virtual int					getSocketCount( void ){ return (int)m_aoSkts.size(); }

	//! close all sockets
	virtual void				closeAllSkts( void );

	//=== functions that should only be called by derived classes ===//
	//! handle callbacks from sockets
	virtual	void				doReceiveCallback( std::shared_ptr<VxSktBase>& sktBase );

	//! add a new socket to manage
	virtual void				addSkt( std::shared_ptr<VxSktBase>& sktBase );
	//! remove a socket from management
	virtual int32_t				removeSkt(	std::shared_ptr<VxSktBase>&	sktBase,				// skt to remove
											bool		bDelete = true );	    // if true delete the skt
	virtual bool				isSktActive( std::shared_ptr<VxSktBase>& sktBase, bool sktMgrLocked = false );

    //! move to erase/delete when safe to do so
    virtual void				moveToEraseList( std::shared_ptr<VxSktBase>& sktBase, bool sktMgrLocked = false );

	// find a socket.. assumes list has been locked
	virtual std::shared_ptr<VxSktBase>	findSktBase( const VxGUID& connectId, bool acceptSktsOnly = false );

    virtual void                dumpSocketStats( const char* reason = nullptr, bool fullDump = false );

    virtual void                setSktLoopback( std::shared_ptr<VxSktBase>& sktLoopback )       { m_SktLoopback = sktLoopback; }

	virtual bool				closeConnection( VxGUID& connectId, ESktCloseReason closeReason );

	virtual void				onOncePer30Seconds( VxGUID& myOnlineId );

    virtual void                sktWasClosed( VxSktBase* sktBase, bool sktMgrLocked );

	virtual void                getSktStatRecords( std::vector<VxSktStatRecord>& sktStatList );

	//=== vars ===//
	int32_t						m_rcLastError{ 0 };
    ESktMgrType					m_eSktMgrType{ eSktMgrTypeNone };   // type of sockets we manage

	VX_SKT_CALLBACK				m_pfnUserReceive{ nullptr };		// receive function must be set by user
	VX_SKT_CALLBACK				m_pfnOurReceive{ nullptr };		    // our receive function to receive Socket states etc
	VX_SKT_CALLBACK				m_pfnUserTransmit{ nullptr };		// receive function may be set by user
	VX_SKT_CALLBACK				m_pfnOurTransmit{ nullptr };		// our transmit function to get Socket transmit stats
	void*						m_pvRxCallbackUserData{ nullptr };	// user defined rx callback data
	void*						m_pvTxCallbackUserData{ nullptr };	// user defined tx callback data

	VX_SKT_MGR_STATUS_CALLBACK  m_pfnSktMgrStatus{ nullptr };
	void*						m_pvSktMgrStatusCallbackUserData{ nullptr };

	UINT						m_uiCreatorVxThreadId{ 0 };		    // thread id of thread that created this object

	InetAddress					m_LclIp;

protected:
    //! delete sockets that have expired
    virtual void                doSktDeleteCleanup( void );
	virtual void				deleteAllSockets( void );

	std::vector<std::shared_ptr<VxSktBase>>		m_aoSkts;					        // array of sockets to manage
	std::vector<std::shared_ptr<VxSktBase>>		m_aoSktsToDelete;			        // skts that will be deleted after 10 sec 
	VxMutex						m_SktMgrMutex;			            // thread mutex
    std::shared_ptr<VxSktBase>	m_SktLoopback;	        
};

