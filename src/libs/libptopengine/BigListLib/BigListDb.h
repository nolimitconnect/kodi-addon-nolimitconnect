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

#include <config_appcorelibs.h>
#include "BigList.h"

#include <CoreLib/DbBase.h>
#include <CoreLib/VxThread.h>

class BigListMgr;
class P2PEngine;

class BigListDb : public DbBase, public BigList
{
public:
	BigListDb() = delete;
	BigListDb( P2PEngine& engine, BigListMgr& bigListMgr );
	virtual ~BigListDb() override = default;

	void						threadedRestoreAll( void );

	//! restore all of given network to lists from database
	int32_t						dbRestoreAll( void );

	bool						isBigListInitialized( void )				{ return m_BigListDbInitialized; }

	std::string					getNetworkKey( void );

	int32_t						dbUpdateSessionTime( VxGUID& onlineId, int64_t lastSessionTime );

	int32_t						removeUserFromDatabase( VxGUID& onlineId );

protected:
	//=== overrides ===//
	//! create tables in database 
    virtual int32_t				onCreateTables( int iDbVersion ) override;
	//! delete tables from database 
    virtual int32_t				onDeleteTables( int oldVersion ) override;

	int32_t						bigListDbStartup( const char* pDbFileName );
	int32_t						bigListDbShutdown( void );

	//! if not in db insert BigListInfo else update database
	int32_t						dbUpdateBigListInfo( BigListInfo * poInfo, const char* networkName );

	//! remove friend by id
	int32_t						dbRemoveBigListInfo( VxGUID& oId );

	//! insert big list info node into database
	int32_t						dbInsertBigListInfoIntoDb( BigListInfo * poInfo, const char* networkName );

	//! update big list info node in database
	int32_t						dbUpdateBigListInfoInDb( BigListInfo * poInfo, const char* networkName );

	//! make big list info into blob
	int32_t						saveBigListInfoIntoBlob( BigListInfo * poInfo, uint8_t * * ppu8RetBlob, int * piRetBlobLen );
	//! restore big list info from blob
	int32_t						restoreBigListInfoFromBlob( uint8_t * pu8Data, int iDataLen, BigListInfo * poInfo, uint64_t lastSessionTime, VxGUID& onlineId );

	//=== vars ===//
	P2PEngine&					m_Engine;
	VxThread					m_BigListLoadThread;			// thread to load nodes from database
	BigListMgr&					m_BigListMgr;

	bool						m_BigListDbInitialized{ false };
	std::string					m_LastNetworkKey;
};
