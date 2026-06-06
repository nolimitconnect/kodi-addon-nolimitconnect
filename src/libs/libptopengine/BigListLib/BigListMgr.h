#pragma once
//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "BigListDb.h"

#include <GuiInterface/IDefs.h>

class IToGui;
class P2PEngine;
class PktAnnList;
class PktAnnounce;
class VxNetIdent;

enum EPktAnnUpdateType
{
	ePktAnnUpdateTypeIgnored,
	ePktAnnUpdateTypeNewContact,
	ePktAnnUpdateTypeContactIsSame,
	ePktAnnUpdateTypeContactChanged,
};

const char* DescribePktAnnUpdateType( EPktAnnUpdateType pktAnnUpdateType );

class P2PEngine;

class BigListMgr : public BigListDb
{
public:
	BigListMgr() = delete;
	BigListMgr( P2PEngine& engine );
	virtual ~BigListMgr() override;

	int32_t						bigListMgrStartup( const char* pDbFileName );
	int32_t						bigListMgrShutdown( void );

	int32_t						updateBigListDatabase( BigListInfo * poInfo, const char* networkName );

	//=== add/remove functions ===//
	//! add a or update remote friend
	EPktAnnUpdateType			updatePktAnn(	PktAnnounce *	poPktAnn, 
												BigListInfo **	ppoRetInfo,
												EHostType		hostType,
												bool			useMyFriendshipFromPktAnn = false,
												bool			useHisFriendshipFromPktAnn = true );	

	bool						updateTempIdent( VxNetIdent& tempIdent ); // return true if new contact

	bool						getFriendships( VxGUID&			hisOnlineId,
												EFriendState&	retMyFriendshipToHim,
												EFriendState&	retHisFriendshipToMe );
	bool						isUserIgnored( VxGUID& hisOnlineId );

	bool						getOnlineName( const VxGUID& hisOnlineId, std::string& onlineName );
    std::string					getOnlineName( const VxGUID& hisOnlineId );
	//! return true if can add friend to list
	bool						canAddFriend( void );
	//! remove from big list.. also from db if bRemoveStorage = true 
	int32_t						removeFriend( PktAnnounce * poPktAnn, bool  bRemoveStorage = true );

	//helpers
	int32_t						FillAnnList(	PktAnnList * poPktAnnList, 
												int iMaxListLen,
												int64_t s64ContactTimeLimitMs,
												bool bIncludeThisNode = false );

	void						LimitListSize( void );

	bool						queryIdent( const VxGUID& onlineId, VxNetIdent& netIdent );

	void						onMyFriendshipChanged( EFriendState prevMyFriendship, VxNetIdent* netIdent );

	bool				        fromGuiDeleteUser( VxGUID& onlineId );

protected:

	//=== vars ===//
    bool                        m_BigListMgrInitialized = false;
};



