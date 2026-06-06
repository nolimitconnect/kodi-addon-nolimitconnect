//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VoiceFeedMgr.h"
#include "PluginBase.h"
#include "PluginMgr.h"
#include "PluginSessionMgr.h"
#include "P2PSession.h"
#include "PluginCamServer.h"

#include <P2PEngine/P2PEngine.h>
#include <MediaProcessor/MediaProcessor.h>

#include <opus/OpusCodec.h>

#include <CoreLib/VxDebug.h>
#include <PktLib/PktVoiceReq.h>
#include <NetLib/VxSktBase.h>

#include <memory.h>

//#define DEBUG_AUTOPLUGIN_LOCK 0 // define to enable lock logging

//============================================================================
VoiceFeedMgr::VoiceFeedMgr( P2PEngine& engine, PluginBase& plugin, PluginSessionMgr& sessionMgr )
: m_Engine( engine )
, m_Plugin( plugin )
, m_PluginMgr( engine.getPluginMgr() )
, m_SessionMgr( sessionMgr )
{
}

//============================================================================
void VoiceFeedMgr::enableAudioCapture( bool enable, VxGUID onlineId )
{
	int prevNeedTxCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	if( !prevNeedTxCount && !enable )
	{
		// nothing to remove
		return;
	}

	if( enable )
	{
		if( !m_Plugin.addVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
		{
            LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s add GUID already in list %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
		}
	}
	else
	{
		if( !m_Plugin.removeVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
		{
			LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s remove GUID not found %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
		}
	}

	int needTxCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	m_Plugin.updateRequestMicrophone( m_Plugin.getPluginType(), prevNeedTxCount, needTxCount );
}

//============================================================================
void VoiceFeedMgr::enableAudioReceive( bool enable, VxGUID onlineId )
{
	int prevNeedRxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	if( !prevNeedRxCount && !enable )
	{
		// nothing to remove
		return;
	}

	if( enable )
	{
		if( !m_Plugin.addVoicePairRx( m_Plugin.getPluginType(), onlineId ) )
		{
			LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s add GUID already in list %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
		}
	}
	else
	{
		if( !m_Plugin.removeVoicePairRx( m_Plugin.getPluginType(), onlineId ) )
		{
			LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s remove GUID not found %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
		}
	}

	int needRxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	m_Plugin.updateRequestMixer( m_Plugin.getPluginType(), prevNeedRxCount, needRxCount );
}

//============================================================================
void VoiceFeedMgr::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	PktVoiceReq* pktReq = (PktVoiceReq*)pktHdr;
	VxGUID srcOnlineId = pktReq->getSrcOnlineId();

	PluginBase::AutoPluginLock autoLock( &m_Plugin );

	auto sessionList = m_SessionMgr.getSessions();
	for( auto iter = sessionList.begin(); iter != sessionList.end(); ++iter )
	{
		PluginSessionBase* poSession = *iter;
		if( srcOnlineId == poSession->getSendToId() && ( poSession->isRxSession() || poSession->isP2PSession() ) )
		{
			if( m_Plugin.isFirstVoicePairRx( m_Plugin.getPluginType(), srcOnlineId ) )
			{
                if( LogEnabled( eLogVoice ) ) LogModule( eLogVoice, LOG_DEBUG, "VoiceFeedMgr::%s skt num %d tmp %d %s", __func__,
                              sktBase->getSktNumber(), sktBase->isTempConnection(), m_Engine.describeUser( srcOnlineId ).c_str() );

				AudioJitterBuffer& jitterBuf = poSession->getJitterBuffer();
				jitterBuf.lockResource();

				char* audioBuf = poSession->getJitterBuffer().getBufToFill();
				if( audioBuf )
				{

					int decodedSamples = poSession->getOpusCodec()->decode( pktReq->getCompressedData(), pktReq->getCompressedDataLen(), (int16_t*)audioBuf, AUDIO_SAMPLES_PER_FRAME );
					if( !(decodedSamples == AUDIO_SAMPLES_PER_FRAME) )
					{
						LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::onPktVoiceReq failed to decode opus" );
					}
				}
				else
				{
					LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::onPktVoiceReq failed to get jitter buffer" );
				}

				//LogMsg( LOG_INFO, "VoiceFeedMgr::onPktVoiceReq jitterBuf.unlockResource\n" );
				jitterBuf.unlockResource();
				break;
			}
			else if( LogEnabled( eLogVoice ) )
			{
				LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s not first rx pair for online id %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
			}
		}
	}
}

//============================================================================
void VoiceFeedMgr::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	PluginBase::AutoPluginLock autoLock( &m_Plugin );

	auto sessionList = m_SessionMgr.getSessions();
	for( auto session : sessionList )
	{
		if( session->isRxSession() || session->isP2PSession() )
		{
			AudioJitterBuffer& jitterBuf = session->getJitterBuffer();
			//LogMsg( LOG_INFO, "VoiceFeedMgr::callbackAudioOutSpaceAvail jitterBuf.lockResource sessionIdx %d\n", sessionIdx );
			jitterBuf.lockResource();
			char* audioBuf = jitterBuf.getBufToRead();
			if( audioBuf )
			{
				//LogMsg( LOG_INFO, "VoiceFeedMgr::callbackAudioOutSpaceAvail playAudio %d\n", sessionIdx );
				m_PluginMgr.getEngine().getMediaProcessor().playAudio( (int16_t*)audioBuf, AUDIO_BUF_SIZE );
				//VxGUID onlineId = iter->first; // local session id
				VxGUID onlineId = session->getSendToId();
				// processor mutex was already locked by call to processor fromGuiAudioOutSpaceAvail which calls callbackAudioOutSpaceAvail
				//LogMsg( LOG_INFO, "VoiceFeedMgr::callbackAudioOutSpaceAvail processFriendAudioFeed %d\n", sessionIdx );
				m_PluginMgr.getEngine().getMediaProcessor().processFriendAudioFeed( onlineId, (int16_t*)audioBuf, AUDIO_BUF_SIZE, true );
			}
			else
			{
				LogModule( eLogVoice, LOG_VERBOSE, "VoiceFeedMgr::callbackAudioOutSpaceAvail no buffer to read" );
			}

			//LogMsg( LOG_INFO, "VoiceFeedMgr::callbackAudioOutSpaceAvail jitterBuf.unlockResource sessionIdx %d\n", sessionIdx );
			jitterBuf.unlockResource();
			//sessionIdx++;
		}
	}
}

//============================================================================
void VoiceFeedMgr::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	if( LogEnabled( eLogVoice ) )
	{
		VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
		LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s from %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
	}
}

//============================================================================
void VoiceFeedMgr::callbackOpusPkt( PktVoiceReq* pktOpusAudio )
{
	PluginBase::AutoPluginLock autoLock( &m_Plugin );

	auto sessionList = m_SessionMgr.getSessions();
	for( auto session : sessionList )
	{
		if( session->isTxSession() || session->isP2PSession() )
		{
			if( m_Plugin.isFirstVoicePairTx( m_Plugin.getPluginType(), session->getSendToId() ) )
			{
				bool result = m_Plugin.txPacket( session->getSendToId(), session->getSkt(), pktOpusAudio );
				if( LogEnabled( eLogVoice ) )
				{
					std::shared_ptr<VxSktBase>& sktBase = session->getSkt();
					LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s skt num %d tmp %d result %d to %s", __func__, 
						sktBase->getSktNumber(), sktBase->isTempConnection(), result, m_Engine.describeUser( session->getSendToId() ).c_str() );
				}

				if( false == result )
				{
					// TODO handle lost connection
				}
			}
			else if( LogEnabled( eLogVoice ) )
			{
				LogModule( eLogVoice, LOG_INFO, "VoiceFeedMgr::%s Plugin %s not first tx pair for id %s", __func__, 
					DescribePluginType( m_Plugin.getPluginType() ), m_Engine.describeUser( session->getSendToId()).c_str());
			}
		}
	}
}

//============================================================================
void VoiceFeedMgr::stopAllSessions( void )
{
	std::vector<VxGUID> onlineIdList;
	m_Plugin.getVoiceTxList( m_Plugin.getPluginType(), onlineIdList );
	PluginBase::AutoPluginLock autoLock( &m_Plugin );
	for( auto& onlineId : onlineIdList )
	{
		enableAudioCapture( false, onlineId );
	}

	m_Plugin.getVoiceRxList( m_Plugin.getPluginType(), onlineIdList );
	for( auto& onlineId : onlineIdList )
	{
		enableAudioReceive( false, onlineId );
	}
}
