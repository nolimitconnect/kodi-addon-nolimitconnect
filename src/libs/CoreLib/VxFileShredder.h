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

#include "VxFileShredderDb.h"
#include "VxThread.h"

class VxFileShredder
{
public:
	VxFileShredder();
	virtual ~VxFileShredder();
	void						initShredder( std::string& dataDirectory );

	void						shredFile( std::string& fileName );

	void						shredFiles( void );
protected:
	void						startThreadIfNotStarted( void );

	//=== vars ===//
	VxFileShredderDb			m_ShredderDb;
	VxMutex						m_ShredListMutex;
	VxThread					m_ShredThread;
	std::vector<std::string>	m_ShredList;
};

VxFileShredder& GetVxFileShredder( void );
