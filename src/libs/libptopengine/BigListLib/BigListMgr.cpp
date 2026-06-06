//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <BigListLib/BigListMgr.h>

#include <BigListLib/BigListInfo.h>
#include <GuiInterface/IToGui.h>
#include <Membership/MemberActiveMgr.h>
#include <Network/StayConnected.h>
#include <Network/NetworkMgr.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxTimeDefs.h>

#include <PktLib/PktAnnList.h>

#include <memory.h>
#include <string.h>
#include <string>

#define BIGLIST_VERSION 0x100

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
const char* DescribePktAnnUpdateType( EPktAnnUpdateType pktAnnUpdateType )
{
	switch( pktAnnUpdateType )
	{
	case ePktAnnUpdateTypeIgnored:
		return "ePktAnnUpdateTypeIgnored";
	case ePktAnnUpdateTypeNewContact:
		return "ePktAnnUpdateTypeNewContact";
	case ePktAnnUpdateTypeContactIsSame:
		return "ePktAnnUpdateTypeContactIsSame";
	case ePktAnnUpdateTypeContactChanged:
		return "ePktAnnUpdateTypeContactChanged";
	default:
		return "PktAnnUpdate Type Unknown";
	}
}

//============================================================================
BigListMgr::BigListMgr( P2PEngine& engine )
: BigListDb( engine, *this )
{	
}

//============================================================================
BigListMgr::~BigListMgr()
{
	bigListMgrShutdown();
}

//============================================================================
//! startup
int32_t BigListMgr::bigListMgrStartup( const char* pDbFileName )
{
	if( m_BigListMgrInitialized )
	{
		bigListMgrShutdown();
	}

    m_BigListMgrInitialized = true;
	return bigListDbStartup( pDbFileName );
}

//============================================================================
//! shutdown
int32_t BigListMgr::bigListMgrShutdown( void )
{
    m_BigListMgrInitialized = false;
	return bigListDbShutdown();
}

//============================================================================
int32_t BigListMgr::updateBigListDatabase( BigListInfo * poInfo, const char* networkName )
{
	int32_t rc = 0;
	if( 0 == poInfo )
	{
		return rc;
	}

	if( m_Engine.shouldInfoBeInDatabase( poInfo ))
	{
		// insert into database
		rc = dbUpdateBigListInfo( poInfo, networkName );
	}
	else
	{
		rc = dbRemoveBigListInfo( poInfo->getMyOnlineId() );
		poInfo->setIsInDatabase( false );
	}

	return rc;
}

//============================================================================
bool BigListMgr::getFriendships( VxGUID& hisOnlineId, EFriendState& retMyFriendshipToHim, EFriendState& retHisFriendshipToMe )
{
	retMyFriendshipToHim = eFriendStateAnonymous;
	retHisFriendshipToMe = eFriendStateAnonymous;

	BigListAutoLock bigListAutoLock( *this );
	BigListInfo* poInfo = findBigListInfo( hisOnlineId, true );	// id of friend to look for
	if( poInfo )
	{
		retMyFriendshipToHim = poInfo->getMyFriendshipToHim();
		retHisFriendshipToMe = poInfo->getHisFriendshipToMe();
		return true;
	}

	return false;
}

//============================================================================
bool BigListMgr::isUserIgnored( VxGUID& hisOnlineId )
{
	EFriendState myFriendshipToHim{ eFriendStateAnonymous };
	EFriendState hisFriendshipToMe{ eFriendStateAnonymous };
	if( getFriendships( hisOnlineId, myFriendshipToHim, hisFriendshipToMe ) )
	{
		return myFriendshipToHim == eFriendStateIgnore;
	}

	return false;
}

//============================================================================
bool BigListMgr::getOnlineName( const VxGUID& hisOnlineId, std::string& onlineName )
{
	BigListAutoLock bigListAutoLock( *this );
	BigListInfo * poInfo = findBigListInfo( hisOnlineId, true );	// id of friend to look for
	if( poInfo )
	{
		onlineName = poInfo->getOnlineName();
		return true;
	}

    onlineName.clear();
	return false;
}

//============================================================================
std::string BigListMgr::getOnlineName( const VxGUID& hisOnlineId )
{
	BigListAutoLock bigListAutoLock( *this );
	BigListInfo * poInfo = findBigListInfo( hisOnlineId, true );	// id of friend to look for
	if( poInfo )
	{
		return poInfo->getOnlineName();
	}

	return std::string();
}

//============================================================================
//! add a or update remote friend.. return true 
EPktAnnUpdateType BigListMgr::updatePktAnn(	PktAnnounce *		poPktAnnIn,	
											BigListInfo **		ppoRetInfo,
											EHostType			hostType,
											bool				useMyFriendshipFromPktAnn,
											bool				useHisFriendshipFromPktAnn )	
{
	EFriendState myFriendship = poPktAnnIn->getMyFriendshipToHim();
	EFriendState hisFriendship = poPktAnnIn->getHisFriendshipToMe();

    // commented out because causes deadlock in toGuiContactAnythingChange which calls P2PEngine::describeUser/getBigListMgr().getOnlineName
    //BigListAutoLock bigListAutoLock( *this );
    bigListLock();
	EPktAnnUpdateType eUpdateType = ePktAnnUpdateTypeContactIsSame;
	bool hostedUserUpdate = IsHostARelayForUsers( hostType ); // update is from host.. do not lower his friendship to you
	bool isMySelf = poPktAnnIn->getMyOnlineId() == m_Engine.getMyOnlineId();
	if( isMySelf )
	{
		poPktAnnIn->setMyFriendshipToHim( eFriendStateFriend );
		poPktAnnIn->setHisFriendshipToMe( eFriendStateFriend );
		LogMsg( LOG_WARNING, "updatePktAnn updating myself" );
	}
	else
	{
		if( IsHostARelayForUsers( hostType ) )
		{
			LogModule( eLogConnect, LOG_VERBOSE, "updatePktAnn updating host %s my friendship %s his friendship %s",
					   DescribeHostType( hostType ), DescribeFriendState( myFriendship ), DescribeFriendState( hisFriendship ) );
		}
		else
		{
			LogModule( eLogConnect, LOG_VERBOSE, "updatePktAnn updating my friendship %s his friendship %s",
					   DescribeFriendState( myFriendship ), DescribeFriendState( hisFriendship ) );
		}
	}

	BigListInfo * poInfo = findBigListInfo( poPktAnnIn->getMyOnlineId(), true );	// id of friend to look for
    bigListUnlock();
	if( poInfo )
	{
		if( !isMySelf )
		{
			if( poInfo->isIgnored() )
			{
				// ignore this person
				poPktAnnIn->setMyFriendshipToHim( eFriendStateIgnore );
				return ePktAnnUpdateTypeIgnored;
			}

			if( poPktAnnIn->getLastSessionTimeMs() > poInfo->getLastSessionTimeMs() )
			{
				poInfo->setLastSessionTimeMs( poPktAnnIn->getLastSessionTimeMs() );
			}
			else
			{
				poPktAnnIn->setLastSessionTimeMs( poInfo->getLastSessionTimeMs() );
			}

			EFriendState hisFriendshipToMe = poPktAnnIn->getHisFriendshipToMe();
			if( !useHisFriendshipFromPktAnn && poInfo->getHisFriendshipToMe() > eFriendStateGuest )
			{
				// use last known friendship. This is for the case that the pkt ann is from host and not directly from user
				hisFriendshipToMe = poInfo->getHisFriendshipToMe();
			}

			bool friendshipChanged = hisFriendshipToMe  != poInfo->getHisFriendshipToMe();
			poInfo->setHisFriendshipToMe( hisFriendshipToMe );
			poPktAnnIn->setHisFriendshipToMe( hisFriendshipToMe );

			// update permission levels to guest if needed
			if( useMyFriendshipFromPktAnn )
			{
				// just assume changed
				poInfo->setMyFriendshipToHim( poPktAnnIn->getMyFriendshipToHim() );
				updateVectorList( poPktAnnIn->getMyFriendshipToHim(), poInfo );
				friendshipChanged = true;
			}
			else
			{
				if( poInfo->isAnonymous() )
				{
					if( hostedUserUpdate || m_Engine.getMemberActiveMgr().isActiveMemberOfAny( poInfo->getMyOnlineId() ) )
					{
						poPktAnnIn->setMyFriendshipToHim( eFriendStateGuest );
						poInfo->makeGuest();
						updateVectorList( eFriendStateGuest, poInfo );
						friendshipChanged = true;
					}
				}
			}

			poPktAnnIn->setMyFriendshipToHim( poInfo->getMyFriendshipToHim() );
			if( friendshipChanged )
			{
				eUpdateType = ePktAnnUpdateTypeContactChanged;
			}

			poPktAnnIn->getOnlineName()[ MAX_ONLINE_NAME_LEN - 1 ] = 0;
			if( 0 != strcmp( poPktAnnIn->getOnlineName(), poInfo->getOnlineName() ) )
			{
				memcpy( poInfo->getOnlineName(), poPktAnnIn->getOnlineName(), MAX_ONLINE_NAME_LEN );
				eUpdateType = ePktAnnUpdateTypeContactChanged;
			}

			poPktAnnIn->getOnlineDescription()[ MAX_ONLINE_DESC_LEN - 1 ] = 0;
			if( 0 != strcmp( poPktAnnIn->getOnlineDescription(), poInfo->getOnlineDescription() ) )
			{
				memcpy( poInfo->getOnlineDescription(), poPktAnnIn->getOnlineDescription(), MAX_ONLINE_DESC_LEN );
				eUpdateType = ePktAnnUpdateTypeContactChanged;
			}

			if( poPktAnnIn->getSearchFlags() != poInfo->getSearchFlags() )
			{
				poInfo->setSearchFlags( poPktAnnIn->getSearchFlags() );
				eUpdateType = ePktAnnUpdateTypeContactChanged;
			}

			if( 0 != memcmp( poPktAnnIn->getPluginPermissions(), poInfo->getPluginPermissions(), PERMISSION_ARRAY_SIZE ) )
			{
				memcpy( poInfo->getPluginPermissions(), poPktAnnIn->getPluginPermissions(), PERMISSION_ARRAY_SIZE );
				eUpdateType = ePktAnnUpdateTypeContactChanged;
			}

			if( ( poPktAnnIn->m_DirectConnectId != poInfo->m_DirectConnectId ) ||
				( poPktAnnIn->m_u8RelayFlags != poInfo->m_u8RelayFlags ) || 
				( poPktAnnIn->getSearchFlags() != poInfo->getSearchFlags() ) ||
				( poPktAnnIn->getSharedFileTypes() != poInfo->getSharedFileTypes() ) )
			{
				eUpdateType = ePktAnnUpdateTypeContactChanged;
			}

            memcpy( (void*)(poInfo->getVxNetIdent()),  (void*)(poPktAnnIn->getVxNetIdent()), sizeof( VxNetIdent ) );

			if( ePktAnnUpdateTypeContactIsSame != eUpdateType )
			{
				m_Engine.toGuiContactAnythingChange( poInfo );
			}

            bigListLock();
			if( m_Engine.shouldInfoBeInDatabase( poInfo ) )
			{
				updateBigListDatabase( poInfo, m_Engine.getNetworkMgr().getNetworkKey().c_str() );
			}
			else
			{
				dbRemoveBigListInfo( poInfo->getMyOnlineId() );
			}

            bigListUnlock();
		}
	}
	else
	{
		if( true == canAddFriend() )
		{
			// new friend
			if( !isMySelf )
			{
				poPktAnnIn->setMyFriendshipToHim( hostedUserUpdate ? eFriendStateGuest : eFriendStateAnonymous );
			}

			poInfo = new BigListInfo();
			memcpy( poInfo, poPktAnnIn, sizeof( PktAnnounce ) );
			//LogMsg( LOG_INFO, "BigListMgr::updatePktAnn: new contact %s Hi 0x%llX, Lo 0x%llX\n", poInfo->getOnlineName(), poInfo->getMyOnlineId().getVxGUIDHiPart(), poInfo->getMyOnlineId().getVxGUIDLoPart() );

			bigInsertInfo( poInfo->getMyOnlineId(), poInfo, true );

            bigListLock();
			if( m_Engine.shouldInfoBeInDatabase( poInfo ) )
			{
				updateBigListDatabase( poInfo, m_Engine.getNetworkMgr().getNetworkKey().c_str() );
			}
			else
			{
				dbRemoveBigListInfo( poInfo->getMyOnlineId() );
			}

             bigListUnlock();
			//! notify new contact found
			eUpdateType = ePktAnnUpdateTypeNewContact;
		}
		else
		{
			LogMsg( LOG_ERROR, "Could not add %s to BigList", poPktAnnIn->getOnlineName() );
			eUpdateType = ePktAnnUpdateTypeIgnored;
		}
	}

	if( ppoRetInfo )
	{
		*ppoRetInfo = poInfo;
	}

	if( ePktAnnUpdateTypeContactIsSame != eUpdateType )
	{
        LogMsg( LOG_DEBUG, "BigListMgr::updatePktAnn %s for %s", DescribePktAnnUpdateType( eUpdateType ),  poInfo->getOnlineName() );
        if( ePktAnnUpdateTypeNewContact == eUpdateType )
        {
            // make sure gui gets the new contact before the online status change is called
            m_Engine.getToGui().toGuiContactAdded( poInfo );
        }
    }

	return eUpdateType;
}

//============================================================================
bool BigListMgr::canAddFriend( void )
{
	// first limit size.. remove old if possible
	LimitListSize();
	if( MAX_BIGLIST_ITEMS > m_BigList.size() )
	{
		return true;
	}
	LogMsg( LOG_ERROR, "BigListMgr::canAddFriend false" );
	return false;
}

//============================================================================
//! remove from big list.. also from db if bRemoveStorage = true 
int32_t BigListMgr::removeFriend( PktAnnounce * poPktAnn, bool  bRemoveStorage )
{
	BigListAutoLock bigListAutoLock( *this );
	int32_t rc = 0;
	bigRemoveInfo( poPktAnn->getMyOnlineId(), true );
	if( bRemoveStorage )
	{
		rc = dbRemoveBigListInfo( *((VxGUID *)poPktAnn) );
	}

	return rc;
}

//============================================================================
int32_t BigListMgr::FillAnnList(	PktAnnList * poPktAnnList, 
								int iMaxListLen,
								int64_t s64ContactTimeLimitMs,
								bool bIncludeThisNode )
{
	BigListAutoLock bigListAutoLock( *this );

	int32_t rc;
	BigListInfo * poInfo;
	int iemptyLen = poPktAnnList->emptyLen();

	std::map< VxGUID, BigListInfo *, cmp_vxguid >::iterator oMapIter;
	for( oMapIter = m_BigList.begin(); oMapIter != m_BigList.end(); ++oMapIter )
	{
		delete oMapIter->second;
		poInfo = oMapIter->second;
		if( poInfo->getElapsedMsTcpLastContact() <= s64ContactTimeLimitMs )
		{
			//is a node we can add if fits
			if( ((int)sizeof( VxNetIdent ) + poPktAnnList->getPktLength() ) <= (iMaxListLen + iemptyLen) )
			{
			
				if( ( false == bIncludeThisNode ) 
					&& ( m_Engine.getMyPktAnnounce().getMyOnlineId() == poInfo->getMyOnlineId() ) )
				{
					// don't include ourself
					continue;
				}
				rc = poPktAnnList->addAnn( poInfo );
				if( rc )
				{
					//list is full
					return -1;
				}
			}
			else
			{
				//filled to limit
				return -1;
			}
		}
	}
	return 0;
}

//============================================================================
void BigListMgr::LimitListSize( void )
{
	return; //NOTE: TODO NEED REWRITTEN>> THIS IS CRAP
	int iCnt = (int)m_BigList.size();
	if( MAX_BIGLIST_ITEMS >= iCnt )
	{
		// list is not to big
		return;
	}

	int iToRemoveCnt = (iCnt - MAX_BIGLIST_ITEMS) + 10; // remove a extra 10 so we don't have to do so often
	int iRemovedCnt = 0;
	BigListInfo * poCurInfo;
	//work the list backwards.. this will tend to put
	//the guests etc at the end of the list
	std::map< VxGUID, BigListInfo *, cmp_vxguid >::iterator oMapIter;
	for( oMapIter = m_BigList.end(); oMapIter != m_BigList.begin(); oMapIter-- )
	{
		poCurInfo = oMapIter->second;
		if( poCurInfo->isSafeToDelete() )
		{
			if( GetGmtTimeMs() - poCurInfo->getElapsedMsAnyContact() > WEEK_OF_MIILISECONDS )
			{
				// has not had contact in week and is not friend etc
				delete poCurInfo;
				m_BigList.erase( oMapIter );
				iRemovedCnt++;
				if( iRemovedCnt >= iToRemoveCnt )
				{
					// we have removed enough
					return;
				}
			}
		}
	}
	// couldn't remove enough.. try again with looser criteria
	for( oMapIter = m_BigList.end(); oMapIter != m_BigList.begin(); oMapIter-- )
	{
		poCurInfo = oMapIter->second;
		if( poCurInfo->isSafeToDelete() )
		{
			if( GetGmtTimeMs() - poCurInfo->getElapsedMsAnyContact() > HOUR_OF_MIILISECONDS * 3 )
			{
				// has not had contact in week and is not friend etc
				delete poCurInfo;
				m_BigList.erase( oMapIter );
				iRemovedCnt++;
				if( iRemovedCnt >= iToRemoveCnt )
				{
					// we have removed enough
					return;
				}
			}
		}
	}
}

//============================================================================
bool BigListMgr::queryIdent( const VxGUID& onlineId, VxNetIdent& netIdent )
{
	bool foundIdent = false;
	if( onlineId.isValid() )
	{
		BigListAutoLock bigListAutoLock( *this );
		auto iter = m_BigList.find( onlineId );
		if( iter != m_BigList.end() )
		{
			BigListInfo* bigListInfo = iter->second;
			if( bigListInfo )
			{
				netIdent = *bigListInfo;
				foundIdent = true;
			}
		}
	}

	return foundIdent;
}

//============================================================================
void BigListMgr::onMyFriendshipChanged( EFriendState prevMyFriendship, VxNetIdent* netIdent )
{
    if( netIdent->getMyOnlineId() == m_Engine.getMyOnlineId() )
    {
        LogMsg( LOG_ERROR, "BigListMgr::onMyFriendshipChanged isMyself true" );
        vx_assert( false );
        return;
    }

    BigListInfo* bigListInfo = findBigListInfo( netIdent->getMyOnlineId() );

    if( bigListInfo )
	{
        updateVectorList( prevMyFriendship, bigListInfo );
	}
	else
	{
		LogMsg( LOG_ERROR, "BigListMgr::onMyFriendshipChanged null BigListInfo for %s %s", 
				netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str() );
		vx_assert( false );
	}
}

//============================================================================
bool BigListMgr::fromGuiDeleteUser( VxGUID& onlineId )
{
	if( onlineId == m_Engine.getMyOnlineId() )
    {
        LogMsg( LOG_ERROR, "BigListMgr::fromGuiDeleteUser cannot delete myself" );
        vx_assert( false );
        return false;
    }

	if( !onlineId.isValid() )
    {
        LogMsg( LOG_ERROR, "BigListMgr::fromGuiDeleteUser invalid guid" );
        vx_assert( false );
        return false;
    }

	BigListInfo* bigListInfo = findBigListInfo( onlineId );
	if( !bigListInfo )
	{
        LogMsg( LOG_ERROR, "BigListMgr::fromGuiDeleteUser user not found %s", onlineId.toOnlineIdString().c_str() );
        vx_assert( false );
        return false;
	}

	bigListInfo->setMyFriendshipToHim( eFriendStateIgnore );
	bool wasDeleted = removeUserFromDatabase( onlineId ) == 0;
	return wasDeleted;
}

//============================================================================
bool BigListMgr::updateTempIdent( VxNetIdent& tempIdent )
{
	bool wasUpdated{ false };
	BigListInfo* bigListInfo = findBigListInfo( tempIdent.getMyOnlineId() );
	if( bigListInfo )
	{
		bigListInfo->setPluginPermissions( tempIdent.getPluginPermissions() );
		bigListInfo->setHisFriendshipToMe( tempIdent.getHisFriendshipToMe() );
		return false;
	}
	else
	{
		BigListInfo* bigListInfo = new BigListInfo();
		memcpy( bigListInfo->getVxNetIdent(), &tempIdent, sizeof( VxNetIdent ) );
		bigListInfo->setMyFriendshipToHim( eFriendStateAnonymous );
		bigListInfo->setNeedSavedToDb( false );
		updateVectorList( eFriendStateAnonymous, bigListInfo );
		return true;
	}
}
