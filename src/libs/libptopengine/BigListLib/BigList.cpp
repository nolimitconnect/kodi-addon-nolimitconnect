//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include <BigListLib/BigList.h>
#include <BigListLib/BigListInfo.h>
#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <CoreLib/VxDebug.h>

#include <string.h>

//============================================================================
void BigList::bigListLock( int iInst )
{ 
#ifdef SHOW_LOCKS
static int lckCnt = 1000;
	if( 0 == iInst )
	{
		lckCnt++;
		iInst = lckCnt;
	}

	LogMsg( LOG_INFO,  "BigList bigListLock %d", iInst );
#endif// SHOW_LOCKS
	m_BigListMutex.lock(__FILE__, __LINE__);
}

//============================================================================
void BigList::bigListUnlock( int iInst )
{
#ifdef SHOW_LOCKS
	static int unlckCnt = 1000;
	if( 0 == iInst )
	{
		unlckCnt++;
		iInst = unlckCnt;
	}

	LogMsg( LOG_INFO,  "BigList bigListUnlock %d", iInst );
#endif// SHOW_LOCKS
	m_BigListMutex.unlock(__FILE__, __LINE__);
}

//============================================================================
//! find contact
BigListInfo* BigList::findBigListInfo( const VxGUID oOnlineId, bool isAlreadyLocked )			// id of contact to look for
{
	if( oOnlineId.isValid() )
	{
		if( false == isAlreadyLocked )
		{
			bigListLock();
		}

		BigListIter oMapIter = m_BigList.find( oOnlineId );
		if( oMapIter != m_BigList.end() )
		{
			BigListInfo * bigListInfo = oMapIter->second;
			if( false == isAlreadyLocked )
			{
				bigListUnlock();
			}

			return bigListInfo;
		}

		if( false == isAlreadyLocked )
		{
			bigListUnlock();
		}
	}

	return NULL;
}

//============================================================================
//! find contact
VxNetIdent* BigList::findNetIdent( VxGUID onlineId, bool isAlreadyLocked )			// id of contact to look for
{
	if( onlineId.isValid() )
	{
		if( onlineId == GetPtoPEngine().getMyOnlineId() )
		{
			return GetPtoPEngine().getMyNetIdent();
		}

		if( false == isAlreadyLocked )
		{
			bigListLock();
		}

		BigListIter oMapIter = m_BigList.find( onlineId );
		if( oMapIter != m_BigList.end() )
		{
			BigListInfo* bigListInfo = oMapIter->second;
			if( false == isAlreadyLocked )
			{
				bigListUnlock();
			}

			return bigListInfo->getVxNetIdent();
		}

		if( false == isAlreadyLocked )
		{
			bigListUnlock();
		}
	}

	return NULL;
}

//============================================================================
//! find contact by name
BigListInfo * BigList::findBigListInfo( const char* pUserName, bool isAlreadyLocked )				// contact to look for
{
	BigListInfo * poInfo;
	BigListIter oMapIter;
	if( false == isAlreadyLocked )
	{
		bigListLock();
	}

	for( oMapIter = m_BigList.begin(); oMapIter != m_BigList.end(); ++oMapIter)
	{
		poInfo = oMapIter->second;
		if( 0 == strcmp(poInfo->getOnlineName(), pUserName) )
		{
			if( false == isAlreadyLocked )
			{
				bigListUnlock();
			}

			return poInfo;
		}
	}

	if( false == isAlreadyLocked )
	{
		bigListUnlock();
	}

	return NULL;
}

//============================================================================
void BigList::bigInsertInfo( VxGUID& oOnlineId, BigListInfo * poInfo, bool isAlreadyLocked )	// contact to insert
{
	//LogMsg(LOG_INFO, " BigList::bigInsertInfo: %s friendship %s isMyRelay %d isMyPreferredRelay %d", 
	//	poInfo->getOnlineName(),
	//	poInfo->describeMyFriendshipToHim(),
	//	poInfo->isMyRelay(),
	//	poInfo->isMyPreferedRelay() );
	if( false == isAlreadyLocked )
	{
		bigListLock();
	}

	m_BigList[ oOnlineId ] = poInfo;
	if( false == isAlreadyLocked )
	{
		bigListUnlock();
	}

	addToVectorList( poInfo->getMyFriendshipToHim(), poInfo );
}

//============================================================================
void BigList::bigRemoveInfo( VxGUID& oOnlineId, bool isAlreadyLocked )			// id of contact to remove
{
	BigListIter oMapIter;
	if( false == isAlreadyLocked )
	{
		bigListLock();
	}

	oMapIter = m_BigList.find( oOnlineId );
	if( oMapIter != m_BigList.end() )
	{
		BigListInfo * poInfo = oMapIter->second;
        GetPtoPEngine().onBigListInfoDelete( poInfo );
		m_BigList.erase( oMapIter );
		if( false == isAlreadyLocked )
		{
				bigListUnlock();
		}

		removeFromVectorList( poInfo->getMyFriendshipToHim(), poInfo );
		delete poInfo;
		return;
	}

	if( false == isAlreadyLocked )
	{
		bigListUnlock();
	}
}

//============================================================================
//! remove everybody in list
void BigList::removeAllInfos( void )
{
	m_AdministratorList.clear();
	m_FriendList.clear();
	m_GuestList.clear();
	m_AnonymousList.clear();
	m_IgnoreList.clear();

	std::map< VxGUID, BigListInfo *, cmp_vxguid >::iterator oMapIter;
	bigListLock();
	for( oMapIter = m_BigList.begin(); oMapIter != m_BigList.end(); ++oMapIter )
	{
		GetPtoPEngine().onBigListInfoDelete( oMapIter->second );
		delete oMapIter->second;
	}

	m_BigList.clear();
	bigListUnlock();
}

//============================================================================
//! update which list person is in
void BigList::updateVectorList( EFriendState oldFriendship,  BigListInfo * poInfo )
{
	LogMsg(LOG_INFO, "updateVectorList: %s old friendship %d", 
			poInfo->getOnlineName(),
			oldFriendship);

	removeFromVectorList( oldFriendship, poInfo );
	addToVectorList( poInfo->getMyFriendshipToHim(), poInfo );
}

//============================================================================
//! remove from whichever vector list person is in
void BigList::removeFromVectorList( EFriendState friendship, BigListInfo *poInfo )
{
	std::vector< BigListInfo * >::iterator iter;
	switch( friendship )
	{
	case eFriendStateAdmin:
		// remove from administrator list
		m_AdministratorListMutex.lock();
		for( iter = m_AdministratorList.begin(); iter != m_AdministratorList.end(); ++iter )
		{
			if( (*iter) == poInfo )
			{
				LogMsg( LOG_INFO, "BigList::removeFromVectorList: administrator %s", poInfo->getOnlineName());
				m_AdministratorList.erase(iter);
				break;
			}
		}

		m_AdministratorListMutex.unlock();
		break;

	case eFriendStateFriend:
		// remove from friend list
		m_FriendListMutex.lock();
		for( iter = m_FriendList.begin(); iter != m_FriendList.end(); ++iter )
		{
			if( (*iter) == poInfo )
			{
				LogMsg( LOG_INFO, "BigList::removeFromVectorList: friend %s", poInfo->getOnlineName());
				m_FriendList.erase(iter);
				break;
			}
		}
		m_FriendListMutex.unlock();
		break;

	case eFriendStateGuest:
		// remove from guest list
		m_GuestListMutex.lock();
		for( iter = m_GuestList.begin(); iter != m_GuestList.end(); ++iter )
		{
			if( (*iter) == poInfo )
			{
				LogMsg( LOG_INFO, "BigList::removeFromVectorList: guest %s", poInfo->getOnlineName());
				m_GuestList.erase(iter);
				break;
			}
		}
		m_GuestListMutex.unlock();
		break;

	case eFriendStateAnonymous:
		// remove from anonymous list
		m_AnonymousListMutex.lock();
		for( iter = m_AnonymousList.begin(); iter != m_AnonymousList.end(); ++iter )
		{
			if( (*iter) == poInfo )
			{
				LogMsg( LOG_INFO, "BigList::removeFromVectorList: anonymous %s", poInfo->getOnlineName());
				m_AnonymousList.erase(iter);
				break;
			}
		}
		m_AnonymousListMutex.unlock();
		break;

	case eFriendStateIgnore:
		// remove from ignore list
		m_IgnoreListMutex.lock();
		for( iter = m_IgnoreList.begin(); iter != m_IgnoreList.end(); ++iter )
		{
			if( (*iter) == poInfo )
			{
				LogMsg( LOG_INFO, "BigList::removeFromVectorList: ignore %s", poInfo->getOnlineName());
				m_IgnoreList.erase(iter);
				break;
			}
		}

		m_IgnoreListMutex.unlock();
		break;

	default:
		LogMsg( LOG_INFO, "BigList::removeFromVectorList: %s unknown friendship %d", 
				poInfo->getOnlineName(), friendship );
	}
}

//============================================================================
//! add to whichever vector list person belongs in
void BigList::addToVectorList( EFriendState friendship, BigListInfo *poInfo )
{
	switch( friendship )
	{
	case eFriendStateAdmin:
		//LogMsg( LOG_INFO, "addToVectorList: administrator %s", poInfo->getOnlineName());
		m_AdministratorListMutex.lock();
		m_AdministratorList.push_back( poInfo );
		m_AdministratorListMutex.unlock();
		break;

	case eFriendStateFriend:
		//LogMsg( LOG_INFO, "addToVectorList: friend %s", poInfo->getOnlineName());
		m_FriendListMutex.lock();
		m_FriendList.push_back( poInfo );
		m_FriendListMutex.unlock();
		break;

	case eFriendStateGuest:
		//LogMsg( LOG_INFO, "addToVectorList: guest %s", poInfo->getOnlineName());
		m_GuestListMutex.lock();
		m_GuestList.push_back( poInfo );
		m_GuestListMutex.unlock();
		break;

	case eFriendStateAnonymous:
		//LogMsg( LOG_INFO, "addToVectorList: anonymous %s", poInfo->getOnlineName());
		m_AnonymousListMutex.lock();
		m_AnonymousList.push_back( poInfo );
		m_AnonymousListMutex.unlock();
		break;

	case eFriendStateIgnore:
		//LogMsg( LOG_INFO, "addToVectorList: ignore %s", poInfo->getOnlineName());
		m_IgnoreListMutex.lock();
		m_IgnoreList.push_back( poInfo );
		m_IgnoreListMutex.unlock();
		break;
	}
}
