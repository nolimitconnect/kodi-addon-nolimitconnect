#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AudioJitterBuffer.h"

#include <OfferBase/OfferBaseInfo.h>

#include <PktLib/VxCommon.h>

#include <CoreLib/VxGUID.h>
#include <CoreLib/VxSha1Hash.h>
#include <CoreLib/VxSemaphore.h>

enum EPluginSessionType
{
	ePluginSessionTypeUnknown,
	ePluginSessionTypeP2P,
	ePluginSessionTypeTx,
	ePluginSessionTypeRx,
	ePluginSessionTypeRelayServer,
	ePluginSessionTypeRelayClient,

	eMaxPluginSessionType
};

class VxSktBase;
class VxPktHdr;
class OpusCodec;

class PluginSessionBase
{
public:
	PluginSessionBase();
	PluginSessionBase( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	PluginSessionBase( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	virtual ~PluginSessionBase();

	virtual void				setPluginType( EPluginType pluginType );
	virtual EPluginType			getPluginType( void );

	void						setSendToId( VxGUID sendToId )				{ m_SendToId = sendToId; }
	VxGUID&						getSendToId( void )							{ return m_SendToId; }

	virtual void				setSkt( std::shared_ptr<VxSktBase>& sktBase );
	virtual std::shared_ptr<VxSktBase>&			getSkt( void );
	virtual void				setSessionType( EPluginSessionType sessionType );
	virtual EPluginSessionType	getSessionType( void );
	virtual OpusCodec *			getOpusCodec( void ); // will create codec if doesn't already exist
	virtual AudioJitterBuffer&  getJitterBuffer( void )						{ return m_JitterBuffer; }

	virtual bool				isP2PSession( void );
	virtual bool				isTxSession( void );
	virtual bool				isRxSession( void );

	virtual void				setIsSessionStarted( bool isStarted );
	virtual bool				getIsSessionStarted( void );

	void						setIsInTest( bool bTest )					{ m_bTest = bTest; }
	bool						isInTest( void )							{ return m_bTest; }

	void						setLclSessionId( VxGUID& lclId )			{ m_LclSessionId = lclId; }
	VxGUID&						getLclSessionId( void )						{ return m_LclSessionId; }
	void						setRmtSessionId( VxGUID& rmtId )			{ m_RmtSessionId = rmtId; }
	VxGUID&						getRmtSessionId( void )						{ return m_RmtSessionId; }

	void						setAssetId( VxGUID& rmtId )					{ m_AssetId = rmtId; }
	VxGUID&						getAssetId( void )							{ return m_AssetId; }

	void						setOfferInfo( OfferBaseInfo& offerInfo, bool isHost );
	OfferBaseInfo&				getOfferInfo( void )						{ return m_OfferInfo; }

	bool 						isHost( void )								{ return m_OfferInfo.getOfferMgr() == eOfferMgrHost; }

	void						setOfferResponse( EOfferResponse eResponse ){ m_eOfferResponse = eResponse; }
	EOfferResponse				getOfferResponse( void )					{ return m_eOfferResponse; }

	bool						waitForTestSemaphore( int iMilliseconds )	{ return m_TestSemaphore.wait(iMilliseconds); }
	void						signalTestSemaphore( void )					{ if(m_bTest) m_TestSemaphore.signal(); }

protected:
	//=== vars ===//
	EPluginType					m_ePluginType{ ePluginTypeInvalid };
	VxGUID						m_SendToId;
	std::shared_ptr<VxSktBase>	m_Skt;
	EPluginSessionType			m_ePluginSessionType{ ePluginSessionTypeUnknown };
	bool						m_bSessionStarted{ false };

	bool						m_bTest{ false };
	VxGUID						m_LclSessionId;
	VxGUID						m_RmtSessionId;
	VxGUID						m_AssetId;
	OfferBaseInfo				m_OfferInfo;
	EOfferResponse				m_eOfferResponse{ eOfferResponseNotSet };
	OpusCodec *					m_OpusCodec{ nullptr };
	AudioJitterBuffer			m_JitterBuffer;

	VxSemaphore					m_TestSemaphore; // semaphore used for synchronous tests
};
