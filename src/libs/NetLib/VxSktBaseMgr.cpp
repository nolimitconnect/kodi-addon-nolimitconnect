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

#include "VxSktAccept.h"
#include "VxSktBaseMgr.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxSktUtil.h>

#include <stdio.h>
#include <memory.h>

namespace
{
	//============================================================================
	void VxSktBaseMgrReceiveFunction(  std::shared_ptr<VxSktBase>& sktBase, void * pvUserData )
	{
		vx_assert( sktBase );
		vx_assert( sktBase->m_SktMgr );
		sktBase->m_SktMgr->doReceiveCallback( sktBase );
	}
}

//============================================================================
VxSktBaseMgr::VxSktBaseMgr()
{
	m_pfnOurReceive = VxSktBaseMgrReceiveFunction;
	m_uiCreatorVxThreadId = VxGetCurrentThreadId();
}

//============================================================================
VxSktBaseMgr::~VxSktBaseMgr()
{
}

//============================================================================
int VxSktBaseMgr::getActiveSktCnt( void )		
{ 
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	lockSktBaseMgr(); 
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	int activeCnt = (int)m_aoSkts.size(); 
	#if defined(DEBUG_SKT_MGR_LOCK)
        LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr activeCnt %d", __func__, activeCnt );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	unlockSktBaseMgr(); return activeCnt; 
}

//============================================================================
int VxSktBaseMgr::getToDeleteSktCnt( void )	
{ 
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	lockSktBaseMgr(); 
	int toDeleteCnt = (int)m_aoSktsToDelete.size(); 
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	#if defined(DEBUG_SKT_MGR_LOCK)
        LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr to delete cnt %d", __func__, toDeleteCnt );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	unlockSktBaseMgr(); 
	return toDeleteCnt; 
}

//============================================================================
// find a socket.. assumes list has been locked
std::shared_ptr<VxSktBase> VxSktBaseMgr::findSktBase( const VxGUID& connectId, bool acceptSktsOnly )
{
    if( connectId.isValid() && ( !acceptSktsOnly || eSktMgrTypeTcpAccept == m_eSktMgrType ) )
    {
        for( auto sktBase : m_aoSkts )
        {
            if( sktBase && sktBase->getSocketId() == connectId )
            {
                return sktBase;
            }
        }
    }

    std::shared_ptr<VxSktBase> nullSktBase;
    return nullSktBase;
}

//============================================================================
void VxSktBaseMgr::sktMgrShutdown( void )
{
	closeAllSkts();
}

//============================================================================
void VxSktBaseMgr::deleteAllSockets()
{
	m_aoSktsToDelete.clear();
}

//============================================================================
void VxSktBaseMgr::setReceiveCallback( VX_SKT_CALLBACK pfnReceive, void* pvUserData )
{
	m_pfnUserReceive = pfnReceive;
	m_pvRxCallbackUserData = pvUserData;
}

//============================================================================
void VxSktBaseMgr::setSktMgrStatusCallback( VX_SKT_MGR_STATUS_CALLBACK pfnSktMgrStatus, void* pvUserData )
{
	m_pfnSktMgrStatus = pfnSktMgrStatus;
	m_pvSktMgrStatusCallbackUserData = pvUserData;
}

//============================================================================
//! add a new socket to manage
void VxSktBaseMgr::addSkt( std::shared_ptr<VxSktBase>& sktBase )
{
	//LogMsg( LOG_INFO, "Adding %s to VxSktBaseMgr skt list\n", sktBase->describeSktType().c_str() );
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	lockSktBaseMgr();
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	m_aoSkts.emplace_back( sktBase );
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	unlockSktBaseMgr();
}

//============================================================================
//! remove a socket from management
int32_t VxSktBaseMgr::removeSkt(  std::shared_ptr<VxSktBase>&	sktBase,		// skt to remove
								bool		bDelete )	// if true delete the skt
{
	int32_t rc = -1;
	if( VxIsAppShuttingDown() )
	{
		return rc;
	}
	
	if( sktBase.get() )
	{
		//LogMsg( LOG_INFO, "Removing Skt ID %d  type %s from VxSktBaseMgr skt list\n", sktBase->getSktNumber(), sktBase->describeSktType().c_str() );
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		lockSktBaseMgr();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
		{
			if( (iter->get()) == sktBase.get() )
			{
				// found it in our list
				LogMsg( LOG_VERBOSE, "%s Deleting Skt ID %d type %s from VxSktBaseMgr skt list", __func__, sktBase->getSktNumber(), sktBase->describeSktType().c_str() );
				m_aoSkts.erase( iter );
				rc = 0;
				break;
			}
		}

		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		unlockSktBaseMgr();
	}
	else
	{
		 LogMsg( LOG_ERROR, "VxSktBaseMgr %s null sktBase", __func__ );
	}

	return rc;
}

//============================================================================
bool VxSktBaseMgr::isSktActive( std::shared_ptr<VxSktBase>& sktBase, bool sktMgrLocked )
{
    if( !sktBase )
    {
        LogMsg( LOG_ERROR, "VxSktBaseMgr::isSktActive null sktBase" );
        return false;
    }

	bool isActive = false;
    vx_assert( sktBase );
    if( sktBase == m_SktLoopback )
    {
        return true;
    }

	if( !sktMgrLocked )
	{
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		lockSktBaseMgr();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
	}

	for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
	{
		if( (*iter) == sktBase )
		{
			// found it in our list
			isActive = true;
			break;
		}
	}

	if( !sktMgrLocked )
	{
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		unlockSktBaseMgr();
	}

	return isActive;
}

//============================================================================
//! Send to all connections
void VxSktBaseMgr::sendToAll( char * pData, int iDataLen, bool sktMgrLocked )	
{
	if( !sktMgrLocked )
	{
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
			lockSktBaseMgr();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
	}

	for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
	{
		std::shared_ptr<VxSktBase>& skt = (*iter);
		if( skt->isTxCryptoKeySet() )
		{
			skt->txEncrypted( pData, iDataLen, true );
		}
		else
		{
			int32_t rc = skt->sendData( pData, iDataLen, true );
			if( skt->isFatalSocketError( rc ) )
			{
				skt->closeSkt( eSktCloseTxFailed, true );
			}
		}
	}

	if( !sktMgrLocked )
	{
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		unlockSktBaseMgr();
	}
}

//============================================================================
//! get number of connected sockets
int VxSktBaseMgr::getConnectedCount( void )
{
	int iConnectedCnt = 0;
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	lockSktBaseMgr();
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
	{
		if( (*iter)->isConnected() )
		{
			iConnectedCnt++;
		}
	}

	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	unlockSktBaseMgr();
	return iConnectedCnt;
}

//============================================================================
//! close all sockets
void VxSktBaseMgr::closeAllSkts( void )
{
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	VxSktBaseMgr::lockSktBaseMgr();
	for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
	{
		if( (*iter)->isConnected() )
		{
            (*iter)->closeSkt(eSktCloseAll, true );
		}
	}

	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	m_aoSkts.clear();
	m_aoSktsToDelete.clear();
	VxSktBaseMgr::unlockSktBaseMgr();
}

//============================================================================
std::shared_ptr<VxSktBase>	VxSktBaseMgr::makeNewSkt( void )					
{ 
	return  std::shared_ptr<VxSktBase>();
}

//============================================================================
std::shared_ptr<VxSktBase> VxSktBaseMgr::makeNewAcceptSkt( void )				
{ 
    std::shared_ptr<VxSktBase> sharedSkt( new VxSktAccept() );
	sharedSkt->setThisSkt( sharedSkt ); // so skt can do callbacks without look up in manager
	return sharedSkt;
}

//============================================================================
void VxSktBaseMgr::doReceiveCallback( std::shared_ptr<VxSktBase>& sktBase )
{
	ESktCallbackReason eCallbackReason = sktBase->getCallbackReason();
	m_pfnUserReceive( sktBase, m_pvRxCallbackUserData );
	if( eSktCallbackReasonClosed == eCallbackReason )
	{
		LogModule( eLogSkt, LOG_VERBOSE, "VxSktBaseMgr::doReceiveCallback: closed %s num %d", sktBase->describeSktType().c_str(), sktBase->getSktNumber() );
	}
}

//============================================================================
void VxSktBaseMgr::handleSktCloseEvent( std::shared_ptr<VxSktBase>& sktBase )
{
    if( sktBase.get() == nullptr )
    {
        LogMsg( LOG_ERROR, "VxSktBaseMgr::%s null sktBase", __func__ );
        return;
    }

    // put this skt in delete list to be deleted later
    moveToEraseList( sktBase );
    doSktDeleteCleanup();
}

//============================================================================
//! delete sockets that have expired
void VxSktBaseMgr::doSktDeleteCleanup()
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

    int64_t timeNowMs = GetGmtTimeMs();
    std::vector<std::shared_ptr<VxSktBase>> deleteSktList;

    bool deletedSkt = true;
    while( deletedSkt )
    {
        std::shared_ptr<VxSktBase> sktToDelete;
        deletedSkt = false;
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        lockSktBaseMgr();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        auto iter = m_aoSktsToDelete.begin();
        // to be deleted sockets delete after 10 seconds
        while( iter != m_aoSktsToDelete.end() )
        {
            sktToDelete = ( *iter );
            if( timeNowMs > sktToDelete->getToDeleteTimeMs() )
            {
                if( sktToDelete->getInUseByRxThread() )
                {
                    LogMsg( LOG_ERROR, "socket id %d handle %d type %s still in use by thread after delete timeout %s", sktToDelete->getSktNumber(), 
                        sktToDelete->getSktHandle(),
                        DescribeSktType( sktToDelete->getSktType() ),
                        sktToDelete->describeSktConnection().c_str() );

                    if( INVALID_SOCKET != sktToDelete->getSktHandle() )
                    {
                        sktToDelete->doCloseThisSocketHandle();
                    }

                    ++iter;
                    break;
                }
                else
                {
                    deleteSktList.emplace_back( sktToDelete );
                    iter = m_aoSktsToDelete.erase( iter );
                    deletedSkt = true;
                    break;
                }
            }
            else
            {
                ++iter;
            }
        }

		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        unlockSktBaseMgr();
    }   

    for( auto sktBase : deleteSktList )
    {
        LogModule( eLogConnect, LOG_VERBOSE, "%s deleting skt %s", __func__, sktBase->describeSktConnection().c_str() );
		sktBase->getThisSkt().reset(); // clear self reference so when the last shared pointer is deleted the socket will be deleted.
		if( sktBase->getThisSkt().use_count() > 1 )
		{
			LogMsg( LOG_ERROR, "*** %s skt %d handle %d still has %d references %s ", __func__, sktBase->getSktNumber(), 
					sktBase->getSktHandle(), sktBase.use_count(), sktBase->describeSktConnection().c_str() );
		}
    }

	deleteSktList.clear();
}

//============================================================================
//! move to erase/delete when safe to do so
void VxSktBaseMgr::moveToEraseList( std::shared_ptr<VxSktBase>& sktBase, bool sktMgrLocked )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

	bool found{ false };
	if( sktBase )
	{
		if( sktBase->getIsInEraseList() )
		{
			return;
		}

        if( !sktMgrLocked )
        {
			#if defined(DEBUG_SKT_MGR_LOCK)
				LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
			#endif // defined(DEBUG_SKT_MGR_LOCK)
            lockSktBaseMgr(); // dont let other threads mess with array while we remove the socket
			#if defined(DEBUG_SKT_MGR_LOCK)
				LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
			#endif // defined(DEBUG_SKT_MGR_LOCK)
        }

		// make sure not allready in the lists to be deleted or will get deleted twice
		for( auto& toDeleteSkt : m_aoSktsToDelete  )
		{
			if( toDeleteSkt.get() == sktBase.get() )
			{
                LogMsg( LOG_ERROR, "%s socket %d handle %d already in to delete list", __func__, sktBase->getSktNumber(), sktBase->getSktHandle() );
                if( !sktMgrLocked )
                {
					#if defined(DEBUG_SKT_MGR_LOCK)
						LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
					#endif // defined(DEBUG_SKT_MGR_LOCK)
                    unlockSktBaseMgr();
                }

				return;
			}
		}

		for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
		{
			if( *iter == sktBase )
			{
				found = true;
				std::shared_ptr<VxSktBase> sktCopy = *iter; // make copy first so destructor is not called before cleanup can be done
				sktCopy->setToDeleteTimeMs( GetGmtTimeMs() + 30000 );
				m_aoSktsToDelete.emplace_back( sktCopy );
				sktCopy->setIsInEraseList( true );
				m_aoSkts.erase( iter );

				sktCopy->shutdownSkt(); // shutdown threads etc.. do not handle packets after this point
				break;
			}
		}

		if( !found )
		{
			LogMsg( LOG_ERROR, "%s socket %d %p was not found in mgr list", __func__, sktBase->getSktNumber(), sktBase.get() );
		}

        if( !sktMgrLocked )
        {
			#if defined(DEBUG_SKT_MGR_LOCK)
				LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
			#endif // defined(DEBUG_SKT_MGR_LOCK)
            unlockSktBaseMgr();
        }
	}
	else
	{
		LogMsg( LOG_ERROR, "%s called with null socket", __func__ );
	}
}

//============================================================================
void VxSktBaseMgr::dumpSocketStats( const char* reason, bool fullDump )
{
    std::string reasonMsg = reason ? reason : "";
	LogModule( eLogSkt, LOG_DEBUG, "%s skt active %zu to delete %zu total in system %d", reasonMsg.c_str(), m_aoSkts.size(), m_aoSktsToDelete.size(), VxSktBase::getCurrentSktCount() );
    if( fullDump )
    {
        int sktCnt = 0;
        uint64_t timeNow = GetTimeStampMs();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        lockSktBaseMgr(); 
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        for( auto& sktBase : m_aoSkts )
        {
            sktCnt++;
            if( sktBase )
            {
                std::string dmpReason = std::to_string( sktCnt ) + " - ";
                if( timeNow - sktBase->getLastActiveTimeMs() > SKT_ALIVE_TIMEOUT )
                {
                    sktBase->dumpSocketStats( dmpReason.c_str(), fullDump );
                }
            }
        }

		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        unlockSktBaseMgr();
    }
}

//============================================================================
bool VxSktBaseMgr::closeConnection( VxGUID& connectId, ESktCloseReason closeReason )
{
	bool wasClosed{ false };
	if( connectId.isValid() )
	{
		std::shared_ptr<VxSktBase> sktBase( nullptr );
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		lockSktBaseMgr();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
		{
			if( ( *iter )->getSocketId() == connectId )
			{
				sktBase = (*iter);
				break;
			}
		}

		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
		unlockSktBaseMgr();
		if( sktBase )
		{
			sktBase->closeSkt( closeReason, true );
			wasClosed = true;
		}
	}

	return wasClosed;
}

//============================================================================
void VxSktBaseMgr::onOncePer30Seconds( VxGUID& myOnlineId )
{
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	lockSktBaseMgr();
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	// we have to make a copy because the skt may be closed/removed from m_aoSkts
	std::vector<std::shared_ptr<VxSktBase>> sktList = m_aoSkts;
	#if defined(DEBUG_SKT_MGR_LOCK)
		LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
	#endif // defined(DEBUG_SKT_MGR_LOCK)
	unlockSktBaseMgr();

    for( auto iter = sktList.begin(); iter != sktList.end(); )
	{
        if( iter->get() == nullptr )
        {
            LogMsg( LOG_ERROR, "%s nullptr in m_aoSkts", __func__ );
            iter = sktList.erase( iter );
        }
        else
        {
            (*iter)->onOncePer30Seconds( myOnlineId, false );
			iter++;
        }
	}
}

//============================================================================
void VxSktBaseMgr::sktWasClosed( VxSktBase* sktBaseIn, bool sktMgrLocked )
{
	if( sktBaseIn->getIsInEraseList() )
	{
		return;
	}

    if( !sktMgrLocked )
    {
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        lockSktBaseMgr();
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s lockSktBaseMgr locked", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
    }

    bool sktFound{false};
    for( auto iter = m_aoSkts.begin(); iter != m_aoSkts.end(); ++iter )
	{
        if(  iter->get() == sktBaseIn )
        {
            moveToEraseList( *iter, true );
            sktFound = true;
			if(LogEnabled(eLogConnect))LogMsg( LOG_DEBUG, "--VxSktBaseMgr::%s moveToEraseList skt num %d ip %s usr %s", __func__,
				sktBaseIn->getSktNumber(), sktBaseIn->getRemoteIp().c_str(), sktBaseIn->describePeerUser().c_str() );
            break;
        }
    }

    if( !sktFound )
    {
        LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s skt not found ip %s usr %s", __func__,
               sktBaseIn->getRemoteIp().c_str(), sktBaseIn->describePeerUser().c_str() );
    }

    if( !sktMgrLocked )
    {
		#if defined(DEBUG_SKT_MGR_LOCK)
			LogMsg( LOG_DEBUG, "VxSktBaseMgr::%s unlockSktBaseMgr", __func__ );
		#endif // defined(DEBUG_SKT_MGR_LOCK)
        unlockSktBaseMgr();
    }
}

//============================================================================
void VxSktBaseMgr::getSktStatRecords( std::vector<VxSktStatRecord>& retSktStatList )
{
	lockSktBaseMgr();
	for( auto sktBase : m_aoSkts )
	{
		retSktStatList.emplace_back( sktBase->getSktStatRecord() );
	}

	unlockSktBaseMgr();
}