//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PushToTalkFeedMgr.h"
#include "PluginBase.h"
#include "PluginMgr.h"
#include "PluginSessionMgr.h"
#include "P2PSession.h"
#include "PluginCamServer.h"

#include <P2PEngine/P2PEngine.h>
#include <PushToTalk/PushToTalkMgr.h>
#include <MediaProcessor/MediaProcessor.h>

#include <opus/OpusCodec.h>

#include <CoreLib/VxDebug.h>
#include <PktLib/PktsPushToTalk.h>
#include <NetLib/VxSktBase.h>

#include <memory.h>

//#define DEBUG_AUTOPLUGIN_LOCK 1

//============================================================================
PushToTalkFeedMgr::PushToTalkFeedMgr( P2PEngine& engine, PluginBase& plugin, PluginSessionMgr& sessionMgr )
: m_Engine( engine )
, m_Plugin( plugin )
, m_PluginMgr( engine.getPluginMgr() )
, m_SessionMgr( sessionMgr )
{
}

//============================================================================
bool PushToTalkFeedMgr::fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk, std::shared_ptr<VxSktBase>& sktBase )
{
	return enableAudioCapture( enableTalk, onlineId, eMediaModulePushToTalk, sktBase );
}

//============================================================================
bool PushToTalkFeedMgr::enableAudioCapture( bool enable, VxGUID& onlineId, EMediaModule mediaModule, std::shared_ptr<VxSktBase>& sktBase )
{
	bool result{ false };
	int prevNeedCnt = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	if( enable )
	{
		if( m_Plugin.addVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
		{
			if( sendPushToTalkReq( onlineId, sktBase ) )
			{
				m_SessionMgr.findOrCreateTxSessionWithOnlineId( onlineId, sktBase, false );
				sendPushToTalkStart( onlineId, sktBase );
				updatePushToTalkStatus( onlineId );
				result = true;
			}
			else
			{
				m_Plugin.removeVoicePairTx( m_Plugin.getPluginType(), onlineId );
				m_Engine.getPushToTalkMgr().pushToTalkStatusChange( onlineId, ePushToTalStatusNoConnection );
				LogModule( eLogVoice, LOG_VERBOSE, "PushToTalkFeedMgr::enableCapture failed sendPushToTalkReq %s", m_Engine.describeUser( onlineId ).c_str() );
			}
		}
		else
		{
            LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::enableCapture true GUID already in list %s", m_Engine.describeUser( onlineId ).c_str() );
		}
	}
	else
	{
		result = true;
		if( m_Plugin.removeVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
		{
			m_SessionMgr.removeTxSessionByOnlineId( onlineId, false );
			sendPushToTalkStop( onlineId, sktBase );
			updatePushToTalkStatus( onlineId );
		}
	}
	
	int needCnt = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	m_Plugin.updateRequestMicrophone( m_Plugin.getPluginType(), prevNeedCnt, needCnt );

    LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::enableCapture %d done %s", enable, m_Engine.describeUser( onlineId ).c_str() );
	return result;
}

//============================================================================
void PushToTalkFeedMgr::callbackAudioOutSpaceAvail( int freeSpaceLenBytes )
{
	#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail PluginBase::AutoPluginLock autoLock start" );
	#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock autoLock( &m_Plugin );
	#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail PluginBase::AutoPluginLock autoLock done" );
	#endif // DEBUG_AUTOPLUGIN_LOCK

	auto sessionList = m_SessionMgr.getSessions();
	for( auto iter = sessionList.begin(); iter != sessionList.end(); ++iter )
	{
		PluginSessionBase* sessionBase = *iter;
		if( sessionBase->isRxSession() || sessionBase->isP2PSession() )
		{
			AudioJitterBuffer& jitterBuf = sessionBase->getJitterBuffer();
			//LogMsg( LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail jitterBuf.lockResource sessionIdx %d\n", sessionIdx );
			jitterBuf.lockResource();
			char* audioBuf = jitterBuf.getBufToRead();
			if( audioBuf )
			{
				//LogMsg( LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail playAudio %d\n", sessionIdx );
				m_PluginMgr.getEngine().getMediaProcessor().playAudio( (int16_t*)audioBuf, AUDIO_BUF_SIZE );
				//VxGUID onlineId = iter->first; // local session id
				VxGUID onlineId = sessionBase->getSendToId();
				// processor mutex was already locked by call to processor fromGuiAudioOutSpaceAvail which calls callbackAudioOutSpaceAvail
				//LogMsg( LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail processFriendAudioFeed %d\n", sessionIdx );
				m_PluginMgr.getEngine().getMediaProcessor().processFriendAudioFeed( onlineId, (int16_t*)audioBuf, AUDIO_BUF_SIZE, true );
			}

			//LogMsg( LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail jitterBuf.unlockResource sessionIdx %d\n", sessionIdx );
			jitterBuf.unlockResource();
		}
	}

	#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogModule( eLogVoice,  LOG_INFO, "PushToTalkFeedMgr::callbackAudioOutSpaceAvail PluginBase::AutoPluginLock autoLock destroy" );
	#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void PushToTalkFeedMgr::onPktPushToTalkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	startRxSession( sktBase, srcOnlineId );
}

//============================================================================
void PushToTalkFeedMgr::onPktPushToTalkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	static int rxCnt{ 0 };
	if( LogEnabled( eLogVoice ) )
	{
		rxCnt++;
		VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
		LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %d from %s", __func__, rxCnt, m_Engine.describeUser( srcOnlineId ).c_str() );
	}
}

//============================================================================
void PushToTalkFeedMgr::onPktPushToTalkStart( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
    if( addPushToTalkUserRx( srcOnlineId, sktBase ) )
	{
        updatePushToTalkStatus( srcOnlineId );
	}

	if( LogEnabled( eLogVoice ) )LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
}

//============================================================================
void PushToTalkFeedMgr::onPktPushToTalkStop( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
    if( removePushToTalkUserRx( srcOnlineId ) )
	{
        updatePushToTalkStatus( srcOnlineId );
	}

	if( LogEnabled( eLogVoice ) )LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
}

//============================================================================
void PushToTalkFeedMgr::onPktVoiceReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    VxGUID srcOnlineId = pktHdr->getSrcOnlineId();
	bool allowed = netIdent->isMyAccessAllowedFromHim( ePluginTypePushToTalk ) && netIdent->isHisAccessAllowedFromMe( ePluginTypePushToTalk );
	if( !allowed )
	{
		// should this be a hack offence?
        LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s user %s insufficient permission", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
		return;
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s PluginBase::AutoPluginLock autoLock start", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock autoLock( &m_Plugin );
#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s PluginBase::AutoPluginLock autoLock done", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK

	bool found{ false };
	auto sessionList = m_SessionMgr.getSessions();
	for( auto iter = sessionList.begin(); iter != sessionList.end(); ++iter )
	{
		PluginSessionBase* poSession = *iter;
        if( poSession->isRxSession() && srcOnlineId == poSession->getSendToId() )
		{
			if( !m_Plugin.isFirstVoicePairRx( m_Plugin.getPluginType(), srcOnlineId ) )
			{
				LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s NOT first user %s", __func__, DescribePluginType( m_Plugin.getPluginType() ), m_Engine.describeUser( srcOnlineId ).c_str() );
				continue;
			}

			found = true;
			// LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s IS first user %s", __func__, DescribePluginType( m_Plugin.getPluginType() ), m_Engine.describeUser( srcOnlineId ).c_str() );
			AudioJitterBuffer& jitterBuf = poSession->getJitterBuffer();
			//LogMsg( LOG_INFO, "PushToTalkFeedMgr::%s jitterBuf.lockResource", __func__ );
			jitterBuf.lockResource();

			char* audioBuf = poSession->getJitterBuffer().getBufToFill();
			if( audioBuf )
			{
				PktVoiceReq* pktReq = (PktVoiceReq*)pktHdr;
				int decodedSamples = poSession->getOpusCodec()->decode( pktReq->getCompressedData(), pktReq->getCompressedDataLen(), (int16_t*)audioBuf, (int32_t)AUDIO_SAMPLES_PER_FRAME );
				if( !(decodedSamples == AUDIO_SAMPLES_PER_FRAME) )
				{
					LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s failed to decode opus", __func__ );
				}
				else
				{
					if( LogEnabled( eLogVoice ) ) LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s decoded from %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
				}
			}
			else
			{
				LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s failed to get jitter buffer", __func__ );
			}

			//LogMsg( LOG_INFO, "PushToTalkFeedMgr::%s jitterBuf.unlockResource" );
			jitterBuf.unlockResource();
			break;
		}
	}

	if( LogEnabled( eLogVoice ) && !found )
	{
		// should this be a hack offence?
		LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s NOT FOUND user %s", __func__, m_Engine.describeUser( srcOnlineId ).c_str() );
		return;
	}

#ifdef DEBUG_AUTOPLUGIN_LOCK
	LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s PluginBase::AutoPluginLock autoLock destroy", __func__ );
#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void PushToTalkFeedMgr::onPktVoiceReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
	onPktPushToTalkReply( sktBase, pktHdr, netIdent );
}

//============================================================================
void PushToTalkFeedMgr::callbackOpusPkt( PktVoiceReq * pktOpusAudio )
{
	#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::callbackOpusPkt PluginBase::AutoPluginLock autoLock start" );
	#endif // DEBUG_AUTOPLUGIN_LOCK
	PluginBase::AutoPluginLock autoLock( &m_Plugin );
	#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::callbackOpusPkt PluginBase::AutoPluginLock autoLock done" );
	#endif // DEBUG_AUTOPLUGIN_LOCK

	auto sessionList = m_SessionMgr.getSessions();
	for( auto& session : sessionList )
	{
		if( session->isTxSession() )
		{
			bool result = m_Plugin.txPacket( session->getSendToId(), session->getSkt(), pktOpusAudio );
			if( LogEnabled( eLogVoice ) )
			{
				if( result )
				{
					LogModule( eLogVoice, LOG_DEBUG, "PushToTalkFeedMgr::%s sent to %s", __func__, m_Engine.describeUser( session->getSendToId() ).c_str() );
				}
				else
				{
					LogModule( eLogVoice, LOG_DEBUG, "PushToTalkFeedMgr::%s failed sent to %s", __func__, m_Engine.describeUser( session->getSendToId() ).c_str() );
				}				
			}

			if( false == result )
			{
				// TODO handle lost connection
			}
		}
	}

	#ifdef DEBUG_AUTOPLUGIN_LOCK
    LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::callbackOpusPkt PluginBase::AutoPluginLock autoLock destroy" );
	#endif // DEBUG_AUTOPLUGIN_LOCK
}

//============================================================================
void PushToTalkFeedMgr::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	cleanupPushToTalkUser( netIdent->getMyOnlineId() );
	if( LogEnabled( eLogVoice ) )LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s", __func__, m_Engine.describeUser( netIdent->getMyOnlineId() ).c_str() );
}

//============================================================================
void PushToTalkFeedMgr::onContactOnlineStatusChange( ConnectId& connectId, bool isOnline )
{
	if( !isOnline )
	{
		VxGUID onlineId = connectId.getUserOnlineId();
		cleanupPushToTalkUser( onlineId );
	}
}

//============================================================================
void PushToTalkFeedMgr::onSessionEnded( VxGUID& onlineId )
{
	cleanupPushToTalkUser( onlineId );
}

//============================================================================
bool PushToTalkFeedMgr::addPushToTalkUserTx( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase )
{
	bool userAdded{ false };

	int prevTxCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	if( m_Plugin.addVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
	{
		userAdded = true;
		m_SessionMgr.findOrCreateTxSessionWithOnlineId( onlineId, sktBase, false );

		int txCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
		m_Plugin.updateRequestMicrophone( m_Plugin.getPluginType(), prevTxCount, txCount );

		LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s add rx from %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	}

	return userAdded;
}

//============================================================================
bool PushToTalkFeedMgr::addPushToTalkUserRx( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase )
{
	bool userAdded{ false };
	int prevRxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	if( m_Plugin.addVoicePairRx( m_Plugin.getPluginType(), onlineId ) )
	{
		userAdded = true;
		m_SessionMgr.findOrCreateRxSessionWithOnlineId( onlineId, sktBase, false );

		int rxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
		m_Plugin.updateRequestMixer( m_Plugin.getPluginType(), prevRxCount, rxCount );

		LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s add tx from %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	}

	return userAdded;
}

//============================================================================
bool PushToTalkFeedMgr::removePushToTalkUserTx( VxGUID& onlineId )
{
	bool userRemoved{ false };
	int prevTxCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	if( m_Plugin.removeVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
	{
		userRemoved = true;
		m_SessionMgr.removeTxSessionByOnlineId( onlineId, false );

		int txCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
		m_Plugin.updateRequestMicrophone( m_Plugin.getPluginType(), prevTxCount, txCount );
	}

	return userRemoved;
}

//============================================================================
bool PushToTalkFeedMgr::removePushToTalkUserRx( VxGUID& onlineId )
{
	bool userRemoved{ false };

	int prevRxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	if( m_Plugin.removeVoicePairRx( m_Plugin.getPluginType(), onlineId ) )
	{
		userRemoved = true;
		m_SessionMgr.removeRxSessionByOnlineId( onlineId, false );

		int rxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
		m_Plugin.updateRequestMixer( m_Plugin.getPluginType(), prevRxCount, rxCount );
	}

	return userRemoved;
}

//============================================================================
bool PushToTalkFeedMgr::sendPushToTalkStart( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase )
{
	LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	PktPushToTalkStart pktStop;
	return m_Plugin.txPacket( onlineId, sktBase, &pktStop );
}

//============================================================================
bool PushToTalkFeedMgr::sendPushToTalkStop( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase )
{
	LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	PktPushToTalkStop pktStop;
	return m_Plugin.txPacket( onlineId, sktBase, &pktStop );
}

//============================================================================
bool PushToTalkFeedMgr::sendPushToTalkReq( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase )
{
	LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	PktPushToTalkReq pktReq;
	return m_Plugin.txPacket( onlineId, sktBase, &pktReq );
}

//============================================================================
void PushToTalkFeedMgr::cleanupPushToTalkUser( VxGUID& onlineId )
{
	int prevTxCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
	if( m_Plugin.removeVoicePairTx( m_Plugin.getPluginType(), onlineId ) )
	{
		int txCount = m_Plugin.needVoiceTxCount( m_Plugin.getPluginType() );
		m_Plugin.updateRequestMicrophone( m_Plugin.getPluginType(), prevTxCount, txCount );
	}

	int prevRxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	if( m_Plugin.removeVoicePairRx( m_Plugin.getPluginType(), onlineId ) )
	{
		int rxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
		m_Plugin.updateRequestMixer( m_Plugin.getPluginType(), prevRxCount, rxCount );
	}

	updatePushToTalkStatus( onlineId );
}

//============================================================================
void PushToTalkFeedMgr::updatePushToTalkStatus( VxGUID& onlineId )
{
	EPushToTalkStatus prevStatus = m_Engine.getPushToTalkMgr().getPushToTalkStatus( onlineId );
	EPushToTalkStatus status{ ePushToTalkStatusInvalid };
	bool isTx = m_Plugin.userNeedsVoicePairTx( m_Plugin.getPluginType(), onlineId );
	bool isRx = m_Plugin.userNeedsVoicePairRx( m_Plugin.getPluginType(), onlineId );
	if( isTx && isRx )
	{
		status = ePushToTalkStatusDuplexEnabled;
	}
	else if( isTx )
	{
		status = ePushToTalkStatusTxEnabled;
	}
	else if( isRx )
	{
		status = ePushToTalkStatusRxEnabled;
	}
	else
	{
		status = ePushToTalkStatusNotActive;
	}

	if( !m_Engine.isUserConnected( onlineId ) )
	{
		status = ePushToTalStatusNoConnection;
	}

    m_Engine.getPushToTalkMgr().pushToTalkStatusChange( onlineId, status );
}

//============================================================================
void PushToTalkFeedMgr::startRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& onlineId )
{
	bool allowed = m_Plugin.isMyAccessAllowedFromHim( onlineId, ePluginTypePushToTalk ) && m_Plugin.isHisAccessAllowedFromMe( onlineId, ePluginTypePushToTalk );
	if( !allowed )
	{
		// should this be a hack offence?
		LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s user %s insufficient permaision", __func__, m_Engine.describeUser( onlineId ).c_str() );
		return;
	}

	int prevRxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	if( m_Plugin.addVoicePairRx( m_Plugin.getPluginType(), onlineId ) )
	{
		m_SessionMgr.findOrCreateRxSessionWithOnlineId( onlineId, sktBase, false );

		updatePushToTalkStatus( onlineId );
	}
	else
	{
		LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s true GUID already in list %s", __func__, m_Engine.describeUser( onlineId ).c_str() );
	}

	int rxCount = m_Plugin.needVoiceRxCount( m_Plugin.getPluginType() );
	if( LogEnabled( eLogVoice ) )LogModule( eLogVoice, LOG_INFO, "PushToTalkFeedMgr::%s prev cnt %d new cnt %d %s", __func__, prevRxCount, rxCount, m_Engine.describeUser( onlineId ).c_str() );
	m_Plugin.updateRequestMixer( m_Plugin.getPluginType(), prevRxCount, rxCount );
}
