//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxHackerMgr.h"

#include <CoreLib/VxDebug.h>

//============================================================================
VxHackerMgr::VxHackerMgr()
{
	VxSetHackReportCallback( this );
}

//============================================================================
VxHackerMgr::~VxHackerMgr()
{
	VxSetHackReportCallback( nullptr );
}

//============================================================================
void VxHackerMgr::reportHackOffense( EHackerLevel hackerLevel, EHackerReason hackerReason, std::string ipAddr, std::string hackDescription )
{
	VxGUID signature;
	std::string sigData = ipAddr + hackDescription;
	if( sigData.length() >= 32 )
	{
		signature.fromRawData( (uint8_t *)sigData.c_str() );
	}

	if( ipAddr.empty() )
	{
		LogModule( eLogHackers, LOG_ERROR, "VxHackerMgr::reportHackOffense no ip address %s", hackDescription.empty() ? "" : hackDescription.c_str() );
	}
	else
	{
		addHackOffense( hackerLevel, hackerReason, ipAddr, signature );
	}
}

//============================================================================
void VxHackerMgr::addHackOffense( EHackerLevel hackerLevel, EHackerReason hackerReason, std::string& ipAddr, VxGUID signature )
{
	if( ipAddr.empty() )
	{
		LogModule( eLogHackers, LOG_ERROR, "VxHackerMgr::addHackOffense no ip address %s %s", DescribeHackerLevel( hackerLevel ),
			DescribeHackerReason( hackerReason ) );
	}
	
	bool foundHacker{ false };

	m_HackListMutex.lock();
	for( auto& hackRecord : m_HackerList )
	{
		if( ipAddr == hackRecord.first )
		{
			foundHacker = true;
			hackRecord.second.incrementHackCount();
			break;
		}
	}

	if( !foundHacker )
	{
		VxHackerRecord hackRecord( hackerLevel, hackerReason, ipAddr, signature );
		m_HackerList[ ipAddr ] = hackRecord;
	}

	m_HackListMutex.unlock();
}

//============================================================================
bool VxHackerMgr::isHacker( std::string& ipAddr )
{
	m_HackListMutex.lock();
	bool isHacker = m_HackerList.find( ipAddr ) != m_HackerList.end();
	m_HackListMutex.unlock();

	return isHacker;
}

//============================================================================
void VxHackerMgr::getHackerList( std::vector<VxHackerRecord>& hackerList )
{
	hackerList.clear();
	m_HackListMutex.lock();
	for( auto& hackRecord : m_HackerList )
	{
		hackerList.push_back( hackRecord.second );
	}

	m_HackListMutex.unlock();
}