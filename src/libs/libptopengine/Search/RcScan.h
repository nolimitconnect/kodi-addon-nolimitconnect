#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RcScanMatchedConnection.h"
#include "RcScanAction.h"

#include <GuiInterface/IScan.h>
#include <PktLib/PktsScan.h>
#include <PktLib/PktsImAlive.h>
#include <CoreLib/VxGUIDList.h>

class PktFindFileReq;
class PktFindFileReply;
class P2PEngine;
class P2PConnectList;

class RcScan : public IScan
{
public:
	RcScan( P2PEngine& engine, P2PConnectList& connectList );
	virtual ~RcScan() = default;

	RcScanAction&				getScanAction( void )		{ return m_ScanAction; }
	EScanType					getScanType( void )			{ return m_ScanAction.getScanType(); }

	void						scanShutdown( void );

	void						fromGuiStartScan( EScanType eScanType, uint8_t searchFlags, uint8_t fileTypeFlags, const char* pSearchPattern = "" );
	void						fromGuiNextScan( EScanType eScanType );
	void						fromGuiStopScan( EScanType eScanType );

	void						onOncePer30Seconds( void );
	void						onOncePerMinute( void );

	void						onIdentDelete( VxNetIdent* netIdent );

	virtual void				onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual void				onConnectionLost( std::shared_ptr<VxSktBase>& sktBase );	
	//! called when new better connection from user
	void						replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt );

	void						onPktScanReq( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, PktScanReq* poPkt );
	void						onPktScanReply( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, PktScanReply * poPkt );
	void						onPktFindFileReq( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, PktFindFileReq* poPkt );
	void						onPktFindFileReply( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, PktFindFileReply * poPkt );

	virtual void				onScanResultProfilePic(	VxNetIdent*	netIdent, 
															std::shared_ptr<VxSktBase>&		sktBase, 
															uint8_t *			pu8JpgData, 
															uint32_t				u32JpgDataLen );

	virtual void				onScanResultError(	EScanType			eScanType,
														VxNetIdent*		netIdent, 
														std::shared_ptr<VxSktBase>&			sktBase, 
														uint32_t					errCode ); 

	void						scanComplete( void );

	void						actionThreadRunning( bool isRunning );

	bool						isLocalSearchMatch( VxNetIdent* netIdent );
	bool						isRemoteSearchMatch( VxNetIdent* netIdent, PktScanReq* poPkt );


protected:
	void						searchMsgToUser( const char* msgToUser, ... );


	P2PEngine&					m_Engine;
	P2PConnectList&				m_ConnectList;

	bool						m_bIsScanning;
	bool						m_bActionThreadRunning;

	EScanType					m_eScanType;

	RcScanAction				m_ScanAction;

	PktScanReq				m_SearchPkt;

	VxGUIDList					m_IdentsReqConnectList; 
	VxGUIDList					m_IdentsSentSearchPktList; 
	VxGUIDList					m_IdentsWithSearchMatchList; 
	VxMutex						m_ScanMutex;

	uint32_t					m_u32ToGuiSendPicTime;
	uint32_t					m_u32SearchActionComplete;
	int64_t						m_s64LastActionTimeMs;
};

