//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include "NetServices/NetServicesMgr.h"
#include "P2PConnectList.h"

#include "Network/NetworkDefs.h" 
#include "Network/NetworkMgr.h"

#include "BigListLib/BigListInfo.h"
#include <Plugins/PluginMgr.h>

#include <CoreLib/VxDebug.h>
#include <NetLib/VxSktBase.h>
#include <PktLib/PktsRelay.h>
#include <NetLib/VxSktCrypto.h>

#include <string.h>

//============================================================================
void P2PEngine::handleTcpData( std::shared_ptr<VxSktBase>& sktBase )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

    if( sktBase->isUdpSocket() )
    {
        LogMsg( LOG_ERROR, "P2PEngine::%s: attempted handle UDP data", __func__ );
        return;
    }

    if( !sktBase->isConnected() )
    {
        LogModule( eLogSktData, LOG_INFO, "P2PEngine::handleTcpData: callback was for data but socket is not connected" );
        return;
    }

	int	iDataLen =	sktBase->getSktBufDataLen();
	if( iDataLen < sizeof(VxPktHdr) || iDataLen & 0x0f )
	{
		return;
	}

	VxGUID firstPktSignature;
	uint32_t u32UsedLen = 0;
	// app should get the buffer ( this also locks it from being modified by threads )
	// then read the data then call sktBufAmountRead ( releases mutex )
    uint8_t * pSktBuf = (uint8_t *)sktBase->getSktReadBuf();
	VxPktHdr* pktHdr = (VxPktHdr*)pSktBuf;
	if( sktBase->isFirstRxPacket() )
	{
        firstPktSignature.fromRawData( pSktBuf );
		if( false == sktBase->isRxEncryptionKeySet() )
		{
			// first packet can be PKT_ANNOUNCE or a NetService req/reply
			// check for NetService first
			bool wasNetServiceRequest{ false };
            bool netServiceRequestRejected{ false };
            bool permissionError{ false };
            uint16_t pktType{0};
            std::string pktTypeDesc;
            std::string cryptoPwd;
            if( getNetServicesMgr().getNetPktRxCryptoPassword( cryptoPwd, sktBase ) )
            {
                std::unique_ptr<VxCrypto> netServCrypto = std::make_unique<VxCrypto>();

                netServCrypto->setPassword( cryptoPwd.c_str(), cryptoPwd.size() );

                // use copy of data because encyption key is different for net services
                uint8_t* bufCopy = new uint8_t[iDataLen];
                memcpy( bufCopy, pSktBuf, iDataLen );
                if( 0 == netServCrypto->decrypt( bufCopy, iDataLen ) )
                {
                    VxPktHdr* pktHdrNetServ = (VxPktHdr*)bufCopy;
                    if( pktHdrNetServ->isValidPktPrefix(false) && pktHdrNetServ->isNetServicePkt() && iDataLen >= pktHdrNetServ->getPktLength() )
                    {
						sktBase->setIsNetServiceConnection( true );
                        wasNetServiceRequest = true;
                        pktType = pktHdrNetServ->getPktType();
                        if( getNetServicesMgr().shouldHandleNetServicePacket() )
                        {                       
                            wasNetServiceRequest = getNetServicesMgr().handlePktNetService( sktBase, pktHdrNetServ, permissionError );
                        }
                        else
                        {
                            netServiceRequestRejected = true;
                            pktTypeDesc = pktHdrNetServ->describePktType( pktType );
							LogMsg( LOG_ERROR, "P2PEngine::%s rejecting net service pkt %s from ip %s", __func__, pktTypeDesc.c_str(), sktBase->getRemoteIp().c_str() );
                        }

						if( permissionError )
						{
							firstPktSignature.fromRawData( pSktBuf );
							hackerOffense( eHackerLevelMedium, eHackerReasonAccessDenied, sktBase->getRemoteIpBinary(), firstPktSignature, "Hacker attempted disabled net service ip %s", sktBase->getRemoteIp().c_str() );
							sktBase->closeSkt( eSktCloseNetServiceHandled );
							delete[] bufCopy;
							return;
						}
						else if( wasNetServiceRequest )
						{
							sktBase->sktBufAmountRead( iDataLen );  // release mutex
							sktBase->setIsFirstRxPacket( false );
							if( netServiceRequestRejected )
							{
								// this can happen if is port open test times out before net service ping request is received
								LogMsg( LOG_DEBUG, "Rejected net service pkt type %s from ip %s", pktTypeDesc.c_str(), sktBase->getRemoteIp().c_str() );
							}

							delete[] bufCopy;
							return;
						}
						else
						{
							// was a valid net service packet that returned false and was not a permission error
							// might have been an incomplete pkt
							LogMsg( LOG_DEBUG, "net service returned false for pkt type %s from ip %s", pktTypeDesc.c_str(), sktBase->getRemoteIp().c_str() );
							delete[] bufCopy;
							return;
						}
                    }
                    //else
                    //{
                    //    LogMsg( LOG_WARN, "P2PEngine::handleTcpData Bad Encryption for Rx net service packet or is really PKT_ANNOUNCE from ip %s", sktBase->getRemoteIp().c_str() );
                    //}
                }

                delete[] bufCopy;
            }
            else
            {
				// could not get crypto pwd for net services
                // make a signature in case it needs to be recorded as hacker
                firstPktSignature.fromRawData( pSktBuf );
            }
		}
		else
		{
			sktBase->decryptReceiveData();
			iDataLen = sktBase->getRxDecryptedLen();
		}

		// not ptop.. better be a announce packet
		if( (false == sktBase->isAcceptSocket()) 
			&& (false == sktBase->isRxEncryptionKeySet() ) )
		{
			// we connected out but never set our key
			sktBase->closeSkt( eSktCloseNoRxEncryptionKey );
			LogMsg( LOG_ERROR, "P2PEngine::%s Accept socket has no rx encryption key", __func__ );
			vx_assert( false );
			return;
		}

        if( false == sktBase->isRxEncryptionKeySet() )
		{
			if( !m_PktAnn.m_DirectConnectId.isIpAddressValid() )
			{
				// the external ip address has not yet been determined
				// just close the connection because we are not ready yet
				sktBase->closeSkt( eSktCloseExternalIpNotDeterminedYet );
				LogMsg( LOG_WARN, "P2PEngine::%s Packet recieved before external ip has been determined", __func__ );
				return;
			}

			// this data has not been decrypted.. set encryption key and decrypt it
			GenerateRxConnectionKey( sktBase, &m_PktAnn.m_DirectConnectId, m_NetworkMgr.getNetworkKey().c_str() );
			sktBase->decryptReceiveData();
			iDataLen = sktBase->getRxDecryptedLen();
		}

		if( ( PKT_TYPE_ANNOUNCE != pktHdr->getPktType() ) ||
			 ( pktHdr->getPktLength() < sizeof( PktAnnounce ) - 100 ) ||  
			 ( pktHdr->getPktLength() > sizeof( PktAnnounce ) + 100 ) ) // leave room for expanding pkt announce in the future
		{
			if( pktHdr->isValidPktPrefix() && pktHdr->getPktType() == PKT_TYPE_PING_REQ )
			{
				// ping request can happen depending on timing.. not really a hack attack so do not block the ip address
				LogMsg( LOG_ERROR, "First packet data len %d is ping request pkt skt %d type %d length %d ip %s:%d id %s signature %s", iDataLen, sktBase->getSktNumber(),
					pktHdr->getPktType(),
					pktHdr->getPktLength(),
					sktBase->getRemoteIp().c_str(),
					sktBase->getRemotePort(),
					firstPktSignature.toOnlineIdString().c_str(),
					firstPktSignature.toAsci().c_str() );

				if( iDataLen != pktHdr->getPktLength() )
				{
					// do not try to recover just close it
					sktBase->closeSkt( eSktClosePktPingRequestIsFirstPkt );
					return;
				}
				else
				{
					// TODO check if we are testing connection
					PktHandlerBase::handlePkt( sktBase, pktHdr );
					return;
				}
			}

			if( pktHdr->isValidPktPrefix() && pktHdr->getPktType() == PKT_TYPE_IM_ALIVE_REQ )
			{
				// ping request can happen depending on timing.. not really a hack attack so do not block the ip address
				LogMsg( LOG_ERROR, "First packet data len %d is im alive request pkt skt %d type %d length %d ip %s:%d id %s signature %s", iDataLen, sktBase->getSktNumber(),
					pktHdr->getPktType(),
					pktHdr->getPktLength(),
					sktBase->getRemoteIp().c_str(),
					sktBase->getRemotePort(),
					firstPktSignature.toOnlineIdString().c_str(),
					firstPktSignature.toAsci().c_str() );

				if( iDataLen != pktHdr->getPktLength() )
				{
					// do not try to recover just close it
					sktBase->closeSkt( eSktClosePktPingRequestIsFirstPkt );
					return;
				}
				else
				{
					// just ignore it
					sktBase->sktBufAmountRead( iDataLen );
					return;
				}
			}

			if( pktHdr->isValidPktPrefix() && pktHdr->getPktType() == PKT_TYPE_HOST_INVITE_ANN_REQ )
			{
				// ping request can happen depending on timing.. not really a hack attack so do not block the ip address
				LogMsg( LOG_ERROR, "First packet data len %d is host invite request pkt skt %d type %d length %d ip %s:%d id %s signature %s", iDataLen, sktBase->getSktNumber(),
					pktHdr->getPktType(),
					pktHdr->getPktLength(),
					sktBase->getRemoteIp().c_str(),
					sktBase->getRemotePort(),
					firstPktSignature.toOnlineIdString().c_str(),
					firstPktSignature.toAsci().c_str() );

				if( iDataLen != pktHdr->getPktLength() )
				{
					// do not try to recover just close it
					sktBase->closeSkt( eSktClosePktPingRequestIsFirstPkt );
					return;
				}
				else
				{
					sktBase->sktBufAmountRead( iDataLen );
					return;
				}
			}

			// somebody tried to send crap .. this may be a hack attack or it may be that our ip and port is same as someone else or network key has changed
			LogMsg( LOG_SEVERE, "First packet data len %d is not Announce pkt skt %d type %d length %d ip %s:%d id %s signature %s", iDataLen, sktBase->getSktNumber(),
																							pktHdr->getPktType(),  
																							pktHdr->getPktLength(),
                                                                                            sktBase->getRemoteIp().c_str(),
																							sktBase->getRemotePort(),
																							firstPktSignature.toOnlineIdString().c_str(),
																							firstPktSignature.toAsci().c_str() );
			// release the skt buffer mutex
			sktBase->sktBufAmountRead( 0 );
            hackerOffense( eHackerLevelMedium, eHackerReasonPktAnnNotFirstPacket, sktBase->getRemoteIpBinary(), firstPktSignature, "Hacker no announcement attack from ip %s", sktBase->getRemoteIp().c_str() );
			sktBase->closeSkt( eSktClosePktAnnNotFirstPacket );
			return;
		}

		if( false == pktHdr->isPktAllHere(iDataLen) )
		{
			// not all of packet arrived
			return;
		}

		// pkt announce has arrived
		PktAnnounce* pktAnn = ( PktAnnounce* )pktHdr;
		if( !pktHdr->isValidPktHdr() || !validateIdent( (VxNetIdent*)pktAnn ) )
		{
			// invalid announcement packet
			sktBase->setIsFirstRxPacket( false ); 
			// release the mutex
			sktBase->sktBufAmountRead( 0 );
            LogMsg( LOG_ERROR, "Invalid Packet announce from ip %s", sktBase->getRemoteIp().c_str() );
			hackerOffense( eHackerLevelMedium, eHackerReasonPktAnnNotFirstPacket, sktBase->getRemoteIpBinary(), firstPktSignature, "Hacker invalid announcement attack from ip %s", sktBase->getRemoteIp().c_str() );
			// disconnect
			sktBase->closeSkt( eSktClosePktAnnInvalid );
		}

		// NOTE: TODO check if is in our Ident ignore list

		//LogMsg( LOG_INFO, "Got Ann on Skt %d\n", sktBase->m_SktNumber );

		u32UsedLen = pktHdr->getPktLength();

		sktBase->setIsFirstRxPacket( false );
		onPktAnnounce( sktBase, pktHdr );
		// success
		// fall thru in case there are more packets
	}

    while( !VxIsAppShuttingDown() )
	{
		//LogMsg( LOG_INFO, "AppRcpCallback.. 3 Skt %d num %d Total Len %d Used Len %d decrypted Len %d\n", 
		//	sktBase->GetSocketId(),
		//	sktBase->m_SktNumber,
		//	u32Len,
		//	u32UsedLen,
		//	sktBase->m_u32DecryptedLen );
		if( false == sktBase->isConnected() )
		{
			//socket has been closed
            LogModule( eLogSktData, LOG_INFO, "P2PEngine::handleTcpData: callback was for data but socket is not connected" );
			return;
		}

		if( sktBase->getRxDecryptedLen() <= u32UsedLen )
		{
			//all decrypted data used up
			break;
		}

		if( sizeof( VxPktHdr ) > ( sktBase->getRxDecryptedLen() - u32UsedLen ) )
		{
			//not enough for a valid packet
			break;
		}

		pktHdr = (VxPktHdr*)&pSktBuf[ u32UsedLen ];
		if( false == pktHdr->isValidPktPrefix() )
		{
			// invalid data
			hackerOffense( eHackerLevelMedium, eHackerReasonPktHdrInvalid, nullptr, sktBase->getRemoteIpBinary(), "Invalid VxPktHdr" );
			// release the mutex
			sktBase->sktBufAmountRead( 0 );
			sktBase->closeSkt( eSktClosePktHdrInvalid );
			return;
		}

		uint16_t u16PktLen = pktHdr->getPktLength();

		if( u32UsedLen + u16PktLen > sktBase->getRxDecryptedLen() )
		{
			//not all of packet is here
			//LogMsg( LOG_VERBOSE,  "AppRcpCallback.. Skt %d num %d Not all of packet arrived\n", 
			//		sktBase->GetSocketId(),
			//		sktBase->m_SktNumber );
			break;
		}

		uint16_t pktType = pktHdr->getPktType();

		if( PKT_TYPE_IM_ALIVE_REQ != pktType && PKT_TYPE_IM_ALIVE_REPLY != pktType && PKT_TYPE_PING_REQ != pktType && PKT_TYPE_PING_REPLY != pktType )
		{
			sktBase->setLastSessionTimeMs( GetGmtTimeMs() );
		}

		if( pktHdr->getDestOnlineId() == getMyOnlineId() )
		{
			PktHandlerBase::handlePkt( sktBase, pktHdr );
		}
		else
		{
            LogModule( eLogRelay, LOG_VERBOSE, "%s Relay Pkt %s from %s ip %s", __func__,
                   pktHdr->describePktHdr().c_str(), sktBase->describePeerUser().c_str(), sktBase->getRemoteIp().c_str() );
			handleIncommingRelayData( sktBase, pktHdr );
		}

		//we used up this packet
		u32UsedLen += u16PktLen;
	}			

	sktBase->sktBufAmountRead( u32UsedLen );
}

//============================================================================
void P2PEngine::handleIncommingRelayData( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	getRelayMgr().handleRelayPkt( sktBase, pktHdr );
}
