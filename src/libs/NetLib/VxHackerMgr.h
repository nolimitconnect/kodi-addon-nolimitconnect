//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include "VxHackerRecord.h"

#include <CoreLib/VxMutex.h>
#include <PktLib/VxCommon.h>

#include <vector>
#include <map>

class VxHackerMgr : public IHackReportCallbackInterface
{
public:
	VxHackerMgr();
	virtual ~VxHackerMgr();

	void						reportHackOffense( EHackerLevel hackerLevel, EHackerReason hackerReason, std::string ipAddr, std::string hackDescription ) override;

	void                        addHackOffense( EHackerLevel hackerLevel, EHackerReason	hackerReason, std::string& ipAddr, VxGUID signature = VxGUID::nullVxGUID() );
	bool						isHacker( std::string& ipAddr );

	void						getHackerList( std::vector<VxHackerRecord>& hackerList );

protected:
	std::map<std::string,VxHackerRecord> m_HackerList;
	VxMutex						m_HackListMutex;
};

