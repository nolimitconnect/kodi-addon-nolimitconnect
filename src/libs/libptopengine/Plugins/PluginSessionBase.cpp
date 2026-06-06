//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginSessionBase.h"

#include <opus/OpusCodec.h>
#include <GuiInterface/IAudioDefs.h>

#include <NetLib/VxSktBase.h>
#include <PktLib/PktsSession.h>

namespace
{
	const int AUDIO_JITTER_QUEUE_DEPTH = 20; // 20 * 60ms = 1.2 seconds of audio buffering
}

//============================================================================
PluginSessionBase::PluginSessionBase()
: m_JitterBuffer( AUDIO_JITTER_QUEUE_DEPTH )
{
}

//============================================================================
PluginSessionBase::PluginSessionBase( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: m_ePluginType( pluginType )
, m_SendToId( sendToId )
, m_Skt( sktBase )
, m_JitterBuffer( AUDIO_JITTER_QUEUE_DEPTH )
{
	m_LclSessionId.initializeWithNewVxGUID();
}

//============================================================================
PluginSessionBase::PluginSessionBase( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType )
: m_ePluginType( pluginType )
, m_SendToId( sendToId )
, m_Skt( sktBase )
, m_LclSessionId( lclSessionId )
, m_eOfferResponse( eOfferResponseNotSet )
, m_JitterBuffer( AUDIO_JITTER_QUEUE_DEPTH )
{
}

//============================================================================
PluginSessionBase::~PluginSessionBase()
{
	delete m_OpusCodec;
}

//============================================================================
OpusCodec *	PluginSessionBase::getOpusCodec( void )		
{ 
	if( nullptr == m_OpusCodec )
	{
		m_OpusCodec = new OpusCodec( AUDIO_DEVICE_SAMPLE_RATE, AUDIO_CHANNELS );
	}

	return m_OpusCodec;
}

//============================================================================
void PluginSessionBase::setPluginType( EPluginType pluginType  )		
{ 
	m_ePluginType = pluginType; 
}

//============================================================================
EPluginType PluginSessionBase::getPluginType( void )
{ 
	return m_ePluginType; 
}

//============================================================================
void PluginSessionBase::setSkt( std::shared_ptr<VxSktBase>& sktBase )		
{ 
	m_Skt = sktBase; 
}

//============================================================================
std::shared_ptr<VxSktBase>& PluginSessionBase::getSkt( void )
{ 
	return m_Skt; 
}

//============================================================================
void PluginSessionBase::setSessionType( EPluginSessionType sessionType )
{
	m_ePluginSessionType = sessionType;
}

//============================================================================
EPluginSessionType PluginSessionBase::getSessionType( void )
{
	return m_ePluginSessionType;
}

//============================================================================
bool  PluginSessionBase::isP2PSession( void )
{
	return m_ePluginSessionType == ePluginSessionTypeP2P ? true : false;
}

//============================================================================
bool  PluginSessionBase::isTxSession( void )
{
	return m_ePluginSessionType == ePluginSessionTypeTx ? true : false;
}

//============================================================================
bool  PluginSessionBase::isRxSession( void )
{
	return m_ePluginSessionType == ePluginSessionTypeRx ? true : false;
}

//============================================================================
void PluginSessionBase::setIsSessionStarted( bool isStarted )	
{ 
	m_bSessionStarted = isStarted; 
}

//============================================================================
bool PluginSessionBase::getIsSessionStarted( void )
{ 
	return m_bSessionStarted; 
}

//============================================================================
void PluginSessionBase::setOfferInfo( OfferBaseInfo& offerInfo, bool isHost )	
{ 
	m_OfferInfo = offerInfo; 
	m_OfferInfo.setOfferMgr( isHost ? eOfferMgrHost : eOfferMgrClient );
}
