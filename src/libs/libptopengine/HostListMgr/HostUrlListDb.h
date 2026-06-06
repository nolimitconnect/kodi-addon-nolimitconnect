#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "HostUrlInfo.h"

#include <CoreLib/DbBase.h>

class HostUrlListDb : public DbBase
{
public:
	HostUrlListDb();
	virtual ~HostUrlListDb() = default;

	int32_t						hostUrlListDbStartup( int dbVersion, const char* dbFileName );
	int32_t						hostUrlListDbShutdown( void );

	void						getAllHostUrls( std::vector<HostUrlInfo>& hostUrlList );
	bool						saveHostUrl( HostUrlInfo& hostUrlInfo );
	void						removeClosedPortIdent( VxGUID& onlineId );

protected:
	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );
};


