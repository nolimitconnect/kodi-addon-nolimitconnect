#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "GroupieInfo.h"

#include <CoreLib/DbBase.h>

class GroupieListDb : public DbBase
{
public:
	GroupieListDb();
	virtual ~GroupieListDb();

	int32_t						groupieListDbStartup( int dbVersion, const char* dbFileName );
	int32_t						groupieListDbShutdown( void );

	void						getAllGroupies( std::vector<GroupieInfo>& groupieList );
	bool						saveGroupie( GroupieInfo& hostedInfo );
	void						removeGroupieInfo( GroupieId& groupieId );
    void						removeGroupieInfo( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType );

	bool                        updateIsFavorite( GroupieId& groupieId, bool isFavorite );
    bool                        updateIsFavorite( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, bool isFavorite );
	bool                        updateLastConnected( GroupieId& groupieId, int64_t lastConnectedTime );
    bool                        updateLastConnected( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, int64_t lastConnectedTime );
	bool                        updateLastJoined( GroupieId& groupieId, int64_t lastJoinedTime );
    bool                        updateLastJoined( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, int64_t lastJoinedTime );
	bool						updateGroupieUrl( bool ipv6, GroupieId& groupieId, std::string& hostUrl );
    bool						updateGroupieUrl( bool ipv6, VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, std::string& groupieUrl );
	bool                        updateGroupieTitleAndDescription( GroupieId& groupieId, std::string& title, std::string& description, int64_t lastDescUpdateTime );
    bool                        updateGroupieTitleAndDescription( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, std::string& title, std::string& description, int64_t lastDescUpdateTime );


protected:
	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

    bool						doesGroupieInfoExist( VxGUID& groupieOnlineId, VxGUID& hostOnlineId, enum EHostType hostType, std::string& retGroupieOnlineHexStr, std::string& retHostOnlineHexStr );
};


