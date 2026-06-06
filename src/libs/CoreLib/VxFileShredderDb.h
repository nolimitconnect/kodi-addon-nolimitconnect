#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "DbBase.h"

class VxFileShredderDb : public DbBase
{
public:
	VxFileShredderDb();
	virtual ~VxFileShredderDb() = default;
	void						initShredderDb( std::string& dataDirectory );

	void						addFileToShred( std::string& fileName );
	void						removeFileToShred( std::string& fileName );

	void						getShredList( std::vector<std::string>&	shredList );

protected:

	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

	//=== vars ===//
};
