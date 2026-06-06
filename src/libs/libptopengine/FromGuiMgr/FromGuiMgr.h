#pragma once
//============================================================================
// Copyright (C) 2023 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/GroupieId.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxSemaphore.h>
#include <CoreLib/VxThread.h>

#include <deque>

// class to avoid stalling the gui thread
// queues from gui action then calls the function from thread

class AssetBaseInfo;
class FromGuiActionBase;
class SearchParams;
class P2PEngine;
class VxNetIdent;
class VxPtopUrl;

class FromGuiMgr
{
public:
	FromGuiMgr( P2PEngine& engine );
	virtual ~FromGuiMgr() = default;

	void						fromGuMgrShutdown( void );

	void						lockFromGuiQue( void )							{ m_FromGuiActionQueMutex.lock(); }
	void						unlockFromGuiQue( void )						{ m_FromGuiActionQueMutex.unlock(); }

	virtual void				fromGuiAppStartup( std::string assetDir, std::string rootDataDir );
    virtual void				fromGuiSetUserSpecificDir( std::string userSpecificDir );
    virtual void				fromGuiSetUserXferDir( std::string userXferDir );
    virtual void				fromGuiUserLoggedOn( VxNetIdent* netIdent );

	virtual void				fromGuiBlockUser( VxGUID& onlineId );

	virtual void				fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl );
    virtual void				fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl );
	virtual void				fromGuiLeaveHost( HostedId& adminId );
	virtual void				fromGuiUnJoinHost( HostedId& adminId );

	virtual void				fromGuiSearchHost( EHostType hostType, SearchParams& searchParams, bool enable );

	virtual void				fromGuiQueryHostListFromNetworkHost( VxPtopUrl& netHostUrl, EHostType hostType, VxGUID& hostIdIfNullThenAll, VxGUID& searchSessionId );

	virtual void				fromGuiPlayOneFrame( AssetBaseInfo& assetInfo );

	void						fromGuiThreadWork( VxThread* workThread );

	void           				fromGuiScanFolderForMedia( VxGUID& appInstId, std::string dirToScan, uint8_t fileTypeFilter );

protected:
	void						queFromGuiAction( FromGuiActionBase* fromGuiAction );

	//=== vars ===//
    P2PEngine&					m_Engine;	

	VxSemaphore					m_FromGuiSemaphore;
	std::deque<FromGuiActionBase*>	m_FromGuiActionQue;
	VxMutex						m_FromGuiActionQueMutex;

	VxThread					m_WorkerThread;
    std::string					m_WorkerThreadName;
};



