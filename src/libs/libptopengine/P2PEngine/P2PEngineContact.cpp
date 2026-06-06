//============================================================================
// Copyright (C) 2009 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <NetServices/NetServicesMgr.h>

#include <BigListLib/BigListInfo.h>
#include <BigListLib/BigList.h>

#include <CoreLib/VxDebug.h>

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

//============================================================================
//! return true if user is viewing this kind of friend
bool P2PEngine::shouldNotifyGui( VxNetIdent* netIdent )
{
	EFriendState eFriendship = netIdent->getMyFriendshipToHim();
	switch( m_eFriendView )
	{
	case eFriendViewEverybody:
		//LogMsg(LOG_INFO, "shouldNotifyGui: should show friend %s" , netIdent->getOnlineName() );
		if( eFriendStateIgnore != eFriendship ) 
		{
			return true;
		}

		break;

	case eFriendViewAdministrators:
		if( eFriendStateAdmin == eFriendship ) 
		{
			return true;
		}

		break;

	case eFriendViewFriendsAndGuests:
		if( ( eFriendStateFriend == eFriendship ) ||
			( eFriendStateGuest == eFriendship ) )
		{
			//LogMsg(LOG_INFO, "shouldNotifyGui: should show friend or guest %s" , netIdent->getOnlineName() );
			return true;
		}
		break;

	case eFriendViewAnonymous:
		if( eFriendStateAnonymous == eFriendship )
		{
			//LogMsg(LOG_INFO, "shouldNotifyGui: should show anon %s" , netIdent->getOnlineName() );
			return true;
		}
		break;

	case eFriendViewIgnored:
		if( eFriendStateIgnore == eFriendship )
		{
			//LogMsg(LOG_INFO, "shouldNotifyGui: should show ignored %s" , netIdent->getOnlineName() );
			return true;
		}
		break;


	default:
		LogMsg(LOG_ERROR, "shouldNotifyGui: UNRECOGNIZED view type" );
	}

	return false;
}

//============================================================================
int P2PEngine::toGuiSendAdministratorList( int iSentCnt, int iMaxSendCnt )
{
	//LogMsg( LOG_INFO, "toGuiSendAdministratorList: called" );
	BigListInfo * poInfo = NULL;
	std::vector< BigListInfo * >::iterator iter;
	m_BigListMgr.m_FriendListMutex.lock(111);
	for( iter = m_BigListMgr.m_AdministratorList.begin(); iter != m_BigListMgr.m_AdministratorList.end(); ++iter )
	{
		poInfo = *iter;
		if( shouldNotifyGui( poInfo ) )
		{
			LogMsg( LOG_INFO, "toGuiSendAdministratorList: %s", poInfo->getOnlineName());
			IToGui::getIToGui().toGuiContactOnline( poInfo->getVxNetIdent() );
		}

		iSentCnt++;
		if( iSentCnt >= iMaxSendCnt )
		{
			break;
		}
	}

	m_BigListMgr.m_FriendListMutex.unlock(111);
	return iSentCnt;
}


//============================================================================
int P2PEngine::toGuiSendFriendList( int iSentCnt, int iMaxSendCnt )
{
	//LogMsg( LOG_INFO, "toGuiSendFriendList: called" );
	BigListInfo * poInfo = NULL;
	std::vector< BigListInfo * >::iterator iter;
	m_BigListMgr.m_FriendListMutex.lock(111);
	for( iter = m_BigListMgr.m_FriendList.begin(); iter != m_BigListMgr.m_FriendList.end(); ++iter )
	{
		poInfo = *iter;
		if( shouldNotifyGui( poInfo ) )
		{
			LogMsg( LOG_INFO, "toGuiSendFriendList: %s" , poInfo->getOnlineName());
			IToGui::getIToGui().toGuiContactOnline( poInfo->getVxNetIdent() );
		}

		iSentCnt++;
		if( iSentCnt >= iMaxSendCnt )
		{
			break;
		}
	}
	
	m_BigListMgr.m_FriendListMutex.unlock(111);
	return iSentCnt;
}

//============================================================================
int P2PEngine::toGuiSendGuestList( int iSentCnt, int iMaxSendCnt )
{
	//LogMsg( LOG_INFO, "toGuiSendGuestList: called" );
	BigListInfo * poInfo = NULL;
	std::vector< BigListInfo * >::iterator iter;
	m_BigListMgr.m_GuestListMutex.lock(112);
	if( iSentCnt < iMaxSendCnt )
	{
		for( iter = m_BigListMgr.m_GuestList.begin(); iter != m_BigListMgr.m_GuestList.end(); ++iter )
		{
			poInfo = *iter;
			if( shouldNotifyGui( poInfo ) )
			{
				LogMsg( LOG_INFO, "toGuiSendGuestList: %s", poInfo->getOnlineName());
				IToGui::getIToGui().toGuiContactOnline( poInfo->getVxNetIdent() );
			}

			iSentCnt++;
			if( iSentCnt >= iMaxSendCnt )
			{
				break;
			}
		}
	}

	m_BigListMgr.m_GuestListMutex.unlock(112);
	return iSentCnt;
}

//============================================================================
int P2PEngine::toGuiSendAnonymousList( int iSentCnt, int iMaxSendCnt )
{
	//LogMsg( LOG_INFO, "toGuiSendAnonymousList: called" );
	BigListInfo * poInfo = NULL;
	std::vector< BigListInfo * >::iterator iter;

	m_BigListMgr.m_AnonymousListMutex.lock(113);
	for( iter = m_BigListMgr.m_AnonymousList.begin(); iter != m_BigListMgr.m_AnonymousList.end(); ++iter )
	{
		poInfo = *iter;
		if( shouldNotifyGui( poInfo ) )
		{
			LogMsg( LOG_INFO, "toGuiSendAnonymousList: %s", poInfo->getOnlineName());
			IToGui::getIToGui().toGuiContactOnline( poInfo->getVxNetIdent() );
		}

		iSentCnt++;
		if( iSentCnt >= iMaxSendCnt )
		{
			break;
		}
	}

	m_BigListMgr.m_AnonymousListMutex.unlock(113);
	return iSentCnt;
}

//============================================================================
int P2PEngine::toGuiSendIgnoreList( int iSentCnt, int iMaxSendCnt )
{
	//LogMsg( LOG_INFO, "toGuiSendIgnoreList: called" );
	BigListInfo * poInfo = NULL;
	std::vector< BigListInfo * >::iterator iter;

	m_BigListMgr.m_IgnoreListMutex.lock(114);
	for( iter = m_BigListMgr.m_IgnoreList.begin(); iter != m_BigListMgr.m_IgnoreList.end(); ++iter )
	{
		poInfo = *iter;
		if( shouldNotifyGui( poInfo ) )
		{
			LogMsg( LOG_INFO, "toGuiSendIgnoreList: %s", poInfo->getOnlineName());
			IToGui::getIToGui().toGuiContactOnline( poInfo->getVxNetIdent() );
		}

		iSentCnt++;
		if( iSentCnt >= iMaxSendCnt )
		{
			break;
		}
	}

	m_BigListMgr.m_IgnoreListMutex.unlock(114);
	return iSentCnt;
}

//============================================================================
//! send all friends for view
void P2PEngine::fromGuiSendContactList( enum EFriendViewType eFriendView, int maxContactsToSend )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiSendContactList" );
	if( m_eFriendView != eFriendView )
	{
		// if view changed then save to settings
		m_eFriendView = eFriendView;
		getEngineSettings().setWhichContactsToView( eFriendView );
	}

	sendToGuiTheContactList( maxContactsToSend );
}

//============================================================================
void P2PEngine::fromGuiRefreshContactList( int maxContactsToSend )
{
	sendToGuiTheContactList( maxContactsToSend );
}

//============================================================================
void P2PEngine::sendToGuiTheContactList( int maxContactsToSend )
{
	int iSentContactsCnt = 0;

	LogMsg( LOG_INFO, "fromGuiSendContactList: view type %d max cnt %d administrators %d friends %d guests %d anon %d ignore %d" ,
			m_eFriendView, 
			maxContactsToSend,
			m_BigListMgr.m_AdministratorList.size(), 
			m_BigListMgr.m_FriendList.size(), 
			m_BigListMgr.m_GuestList.size(),
			m_BigListMgr.m_AnonymousList.size(),
			m_BigListMgr.m_IgnoreList.size() );
	switch( m_eFriendView )
	{
	case eFriendViewEverybody:
		// send friends first then guests
		//LogMsg( LOG_INFO, "fromGuiSendContactList: sending all"  );
		iSentContactsCnt += toGuiSendAdministratorList( iSentContactsCnt, maxContactsToSend );
		iSentContactsCnt += toGuiSendFriendList( iSentContactsCnt, maxContactsToSend );
		iSentContactsCnt += toGuiSendGuestList( iSentContactsCnt, maxContactsToSend );
		iSentContactsCnt += toGuiSendAnonymousList( iSentContactsCnt, maxContactsToSend );
		iSentContactsCnt += toGuiSendIgnoreList( iSentContactsCnt, maxContactsToSend );
		break;

	case eFriendViewAdministrators:
		// send friends first then guests
		//LogMsg( LOG_INFO, "fromGuiSendContactList: sending administrators"  );
		iSentContactsCnt += toGuiSendAdministratorList( iSentContactsCnt, maxContactsToSend );
		break;

	case eFriendViewFriendsAndGuests:
		//LogMsg( LOG_INFO, "fromGuiSendContactList: sending friends and guests"  );
		// send friends first then guests
		iSentContactsCnt += toGuiSendFriendList( iSentContactsCnt, maxContactsToSend );
		iSentContactsCnt += toGuiSendGuestList( iSentContactsCnt, maxContactsToSend );
		break;

	case eFriendViewAnonymous:
		//LogMsg( LOG_INFO, "fromGuiSendContactList: sending anon"  );
		iSentContactsCnt = toGuiSendAnonymousList( iSentContactsCnt, maxContactsToSend );
		break;

	case eFriendViewIgnored:
		//LogMsg( LOG_INFO, "fromGuiSendContactList: sending ignored"  );
		iSentContactsCnt = toGuiSendIgnoreList( iSentContactsCnt, maxContactsToSend );
		break;

	default:
		LogMsg( LOG_INFO, "fromGuiSendContactList: unknown view type %d" , (int)m_eFriendView );
	}


	//LogMsg( LOG_INFO, "fromGuiSendContactList: total sent %d" , iSentContactsCnt);
}

//============================================================================
void P2PEngine::sendToGuiStatusMessage( const char* statusMsg, ... )
{
	char as8Buf[ 1024 ];
	va_list argList;
	va_start( argList, statusMsg );
	vsnprintf( as8Buf, sizeof( as8Buf ), statusMsg, argList );
	as8Buf[sizeof( as8Buf ) - 1] = 0;
	va_end( argList );
	getToGui().toGuiStatusMessage( as8Buf );
}

//============================================================================
//! called when any contact info changes ( including any of the above )
void P2PEngine::toGuiContactAnythingChange( VxNetIdent* netIdent )
{
	if( shouldNotifyGui( netIdent ) )
	{
		if(LogEnabled(eLogUsers))LogModule( eLogUsers, LOG_INFO, "P2PEngine::%s user %s", __func__,
			describeUser(netIdent->getMyOnlineId() ).c_str());
		IToGui::getIToGui().toGuiContactAnythingChange( netIdent );
	}
}



