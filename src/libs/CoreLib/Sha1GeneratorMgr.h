#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "Sha1GeneratorCallback.h"
#include "Sha1ClientInfo.h"
#include "VxThread.h"
#include "VxMutex.h"

#include <vector>

class Sha1GeneratorMgr
{
public:
	Sha1GeneratorMgr();
	virtual ~Sha1GeneratorMgr() = default;

	void						generateSha1( VxGUID& fileId, std::string& fileName, std::string& fileNameAndPath, Sha1GeneratorCallback* client );
	void						cancelGenerateSha1( VxGUID& fileId, std::string& fileNameAndPath, Sha1GeneratorCallback* client );

	void						threadGenerateSha1( VxThread* vxThread );

	void						announceResult( ESha1GenResult sha1GenResult, Sha1ClientInfo& sha1Info );

protected:
	void						startThreadIfNotStarted( void );

	//=== vars ===//
	VxMutex						m_Sha1ListMutex;
	VxThread					m_Sha1Thread;
	std::vector<Sha1ClientInfo>	m_Sha1List;
};

Sha1GeneratorMgr& GetSha1GeneratorMgr( void );
