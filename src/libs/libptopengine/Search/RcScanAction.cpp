
//============================================================================
// Copyright (C) 2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RcScanAction.h"
#include "RcScanPic.h"
#include "RcScan.h"

#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <Plugins/PluginCamServer.h>
#include <Plugins/PluginMgr.h>

#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>
#include <NetLib/VxSktBase.h>

namespace
{
	#define MAX_PICS_IN_SCAN_QUE				2
	#define TIMEOUT_WAIT_FOR_PROFILE_PIC	    8
}

//============================================================================
//! thread function to load all nodes in big list
void * SearchActionThreadFunction( void * pvParam )
{
	VxThread* poThread = (VxThread*)pvParam;
	poThread->setIsThreadRunning( true );
	RcScanAction * poScan = (RcScanAction *)poThread->getThreadUserParam();
    if( poScan && false == poThread->isAborted() )
    {
        poScan->doSearchResultActions();
    }

	poThread->threadAboutToExit();
    return nullptr;
}

//============================================================================
RcScanAction::RcScanAction( P2PEngine& engine, RcScan& oScan )
: m_Engine( engine )
, m_Scan( oScan ) 
, m_bNextScan( true )
, m_SearchConnectionsTimedOut( false )
{
}

//============================================================================
void RcScanAction::fromGuiStartScan( EScanType eScanType )
{
	m_eScanType						= eScanType;
	m_bNextScan						= true;
	m_SearchConnectionsTimedOut		= false;

	if( ( eScanTypePeopleSearch == m_eScanType ) 
		|| ( eScanTypeMoodMsgSearch == m_eScanType  )
		|| ( eScanTypeProfilePic == m_eScanType  )
		|| ( eScanTypeCamServer == m_eScanType  ) )
	{
		// don't need a thread .. we just forward the connections
		m_Scan.actionThreadRunning( false );
	}
	else
	{
		startSearchActionThread();
	}
}

//============================================================================
void RcScanAction::fromGuiStopScan( EScanType eScanType )
{
	stopSearchActionThread();
	m_eScanType = eScanTypeNone;
	cleanupScanResources();
}

//============================================================================
void RcScanAction::fromGuiNextScan( EScanType eScanType )
{
	m_bNextScan = true;
	m_SearchActionSemaphore.signal();
}

//============================================================================
void RcScanAction::onOncePer30Seconds( void )
{

}

//============================================================================
void RcScanAction::onOncePerMinute( void )
{
	// keep connections alive even if not friends
                /*
    std::vector<RcScanMatchedConnection>::iterator iter;
	m_SearchActionMutex.lock();
	for( iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.end(); ++iter )
	{
		RcScanMatchedConnection& matchedConn =  (*iter);
		if( false == matchedConn.getIsActionCompleted() )
        {
			if( false == m_Engine.txImAlivePkt( matchedConn.m_Ident->getMyOnlineId(), matchedConn.m_Skt ) )
			{
                // just mark as completed so dont try again
				matchedConn.setIsActionCompleted( true );
			}
		}
	}

	m_SearchActionMutex.unlock();
            */
}

//============================================================================
void RcScanAction::searchConnectionsTimedOut( void )
{
	m_SearchConnectionsTimedOut = true;
	if( 0 == m_MatchedConnectionsList.size() )
	{
		m_Scan.scanComplete();
	}
}

//============================================================================
void RcScanAction::addMatchedConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	/* TODO reimplement scan
	m_SearchActionMutex.lock();
	m_MatchedConnectionsList.push_back( RcScanMatchedConnection( netIdent, sktBase ) );
	if( eScanTypeProfilePic == m_eScanType )
	{
		RcScanMatchedConnection * matchedConn = findMatchedConnection( netIdent );
		if( matchedConn )
		{
			matchedConn->setActionStartTimeMs( GetGmtTimeMs() );
			PluginBaseWebServer * poPlugin = (PluginBaseWebServer *)m_Engine.getPluginMgr().getPlugin( ePluginTypeWebServer );
			if( false == poPlugin->searchActionProfilePic( netIdent, sktBase ) )
			{
				LogMsg( LOG_ERROR, "RcScanAction::fetchProfilePic failed pic for user %s", netIdent->getOnlineName() );
				matchedConn->setActionHadError( true );
				m_SearchActionMutex.unlock();
				removeIdent( netIdent );
				return;
			}
			else
			{
				LogMsg( LOG_INFO, "RcScanAction::fetchProfilePic success searchActionProfilePic user %s", netIdent->getOnlineName() );
			}
		}

		m_SearchActionMutex.unlock();
	}
	else
	{
		m_SearchActionMutex.unlock();
		m_SearchActionSemaphore.signal();
	}
	*/
}

//============================================================================
void RcScanAction::doSearchResultActions( void )
{
	m_Scan.actionThreadRunning( true );
	while( ( false == VxIsAppShuttingDown() )
			&& ( false == m_SearchActionThread.isAborted() ) )
	{
		if( eScanTypeFileSearch == m_eScanType )
		{
			// todo implement
		}


		if( m_SearchActionThread.isAborted() )
		{
			break;
		}

		VxSleep( 1000 );
	}

	m_Scan.actionThreadRunning( false );
}

//============================================================================
bool RcScanAction::getNextActionConnection( VxNetIdent** ppoIdent, std::shared_ptr<VxSktBase>& ppoSkt )
{
	while( m_MatchedConnectionsList.size() )
	{
		bool validConnection = false;
		m_SearchActionMutex.lock();
		RcScanMatchedConnection& matchedConn = m_MatchedConnectionsList[0];
		if( ( false == matchedConn.getIsActionCompleted() )
			&& matchedConn.m_Skt->isConnected() )
		{
			* ppoIdent	= matchedConn.m_Ident;
			ppoSkt	= matchedConn.m_Skt;
			validConnection = true;
		}

		matchedConn.deleteResources();
		m_MatchedConnectionsList.erase( m_MatchedConnectionsList.begin() );
		m_SearchActionMutex.unlock();
		if( validConnection )
		{
			return true;
		}
	}

	return false;
}


//============================================================================
void RcScanAction::nextCamServerToGui()
{
	if( getShouldSendNext() )
	{
		VxNetIdent* netIdent;
		std::shared_ptr<VxSktBase> sktBase( nullptr );
		while( getNextActionConnection( &netIdent, sktBase ) )
		{
			//if( searchActionWebCamServer( netIdent, sktBase) )
			{
				setShouldSendNext( false );
				break;
			}
		}
	}
}


//============================================================================
void RcScanAction::searchActionPeopleSearch( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	// send this person to gui
}

//============================================================================
void RcScanAction::searchActionFileSearch( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{

}

//============================================================================
void RcScanAction::cleanupScanResources( void )
{
	std::vector<RcScanMatchedConnection>::iterator iter;
	m_SearchActionMutex.lock();
	for( iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.end(); ++iter ) 
	{
		(*iter).deleteResources();
	}	

	m_MatchedConnectionsList.clear();
	m_SearchActionMutex.unlock();
}

//============================================================================
RcScanMatchedConnection *  RcScanAction::findMatchedConnection( VxNetIdent* netIdent )
{
	std::vector<RcScanMatchedConnection>::iterator iter;
	for( iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.end(); ++iter ) 
	{
		if( (*iter).m_Ident->getMyOnlineId().isEqualTo( netIdent->getMyOnlineId() ) )
		{
			return &(*iter);
		}
	}	

	return 0;
}

//============================================================================
void RcScanAction::onScanResultError(	EScanType			eScanType,
										VxNetIdent*		netIdent,
										std::shared_ptr<VxSktBase>&			sktBase,  
										uint32_t					errCode )
{
	removeIdent( netIdent );
}

//============================================================================
void RcScanAction::onScanResultProfilePic(	VxNetIdent*	netIdent, 
												std::shared_ptr<VxSktBase>&		sktBase, 
												uint8_t *			pu8JpgData, 
												uint32_t				u32JpgDataLen )
{
	//m_Engine.getToGui().toGuiSearchResultProfilePic( netIdent, pu8JpgData, u32JpgDataLen );
	removeIdent( netIdent );
}

//============================================================================
// handle case where BigListInfo is about to be deleted
void RcScanAction::onIdentDelete( VxNetIdent* netIdent )
{
	removeIdent( netIdent );
}

//============================================================================
void RcScanAction::removeIdent( VxNetIdent* netIdent )
{
	std::vector<RcScanMatchedConnection>::iterator iter;
	m_SearchActionMutex.lock();
	for( iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.end(); ++iter ) 
	{

		if( (*iter).m_Ident->getMyOnlineId().isEqualTo( netIdent->getMyOnlineId() )  )
		{
			(*iter).deleteResources();
			m_MatchedConnectionsList.erase(iter);
			break;
		}
	}	

	m_SearchActionMutex.unlock();
}

//============================================================================
void RcScanAction::onContactWentOnline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
}

//============================================================================
void RcScanAction::onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
{
	std::vector<RcScanMatchedConnection>::iterator iter;
	m_SearchActionMutex.lock();
	for( iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.end(); ++iter ) 
	{

		if( (*iter).m_Ident->getMyOnlineId().isEqualTo( netIdent->getMyOnlineId() )  )
		{
			m_MatchedConnectionsList.erase(iter);
			break;
		}
	}	

	m_SearchActionMutex.unlock();
}

//============================================================================
//! called when connection is lost
void RcScanAction::onConnectionLost( std::shared_ptr<VxSktBase>& sktBase )
{
	m_SearchActionMutex.lock();
 
	for( auto iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.end(); ) 
	{
		if( (*iter).m_Skt == sktBase )
		{
			iter = m_MatchedConnectionsList.erase(iter);
			break;
		}
		else
		{
			++iter;
		}
	}	

	m_SearchActionMutex.unlock();
}

//============================================================================
//! called when new better connection from user
void RcScanAction::replaceConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& poOldSkt, std::shared_ptr<VxSktBase>& poNewSkt )
{
	std::vector<RcScanMatchedConnection>::iterator iter;
	m_SearchActionMutex.lock();
	for( iter = m_MatchedConnectionsList.begin(); iter != m_MatchedConnectionsList.begin(); ++iter )
	{
		if( (*iter).m_Skt == poOldSkt )
		{
			(*iter).m_Skt = poNewSkt;
		}
	}

	m_SearchActionMutex.unlock();
}

//============================================================================
void RcScanAction::startSearchActionThread( void )
{
	m_SearchActionThread.startThread( (VX_THREAD_FUNCTION_T)SearchActionThreadFunction, this, "SearchAction" );
	m_SearchActionThread.setIsThreadRunning( true );
}

//============================================================================
void RcScanAction::stopSearchActionThread( void )
{
	if( m_SearchActionThread.isThreadRunning() )
	{
		m_SearchActionThread.abortThreadRun( true );
		m_SearchActionSemaphore.signal();
		m_SearchActionThread.killThread();
	}
}
