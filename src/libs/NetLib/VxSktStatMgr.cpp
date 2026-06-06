//============================================================================
// Copyright (C) 2022 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktStatMgr.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxSktUtil.h>

namespace
{
    const int MAX_OFFENSES = 3;
}

//============================================================================
VxSktStatMgr::VxSktStatMgr()
{
	VxSetSktStatCallback( this );
}

//============================================================================
VxSktStatMgr::~VxSktStatMgr()
{
	VxSetSktStatCallback( nullptr );
}

//============================================================================
void VxSktStatMgr::getSktStatRecords( std::vector<VxSktStatRecord>& retSktStatList )
{
	retSktStatList.clear();
	m_SktStatMutex.lock();
	for( auto& sktPair : m_SktStatList )
	{
		retSktStatList.push_back( sktPair.second );
	}

	m_SktStatMutex.unlock();
}


//============================================================================
void VxSktStatMgr::sktConnected( SOCKET skt )
{
	if( skt <= 0 )
	{
		LogMsg( LOG_ERROR, "VxSktStatMgr::sktConnected invalid socket %d", skt );
		return;
	}

	m_SktStatMutex.lock();
	if( m_SktStatList.find( skt ) == m_SktStatList.end() )
	{
		VxSktStatRecord sktStat( skt );
		m_SktStatList[ skt ] = sktStat;
	}
	//else
	//{
	//	LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktConnected connected on existing socket %d", skt );
	//}

	m_SktStatMutex.unlock();
	LogModule( eLogConnect, LOG_INFO, "VxSktStatMgr::sktConnected skt %d connected cnt %zu", skt, m_SktStatList.size() );
}

//============================================================================
void VxSktStatMgr::sktConnected2( SOCKET skt, std::string ipAddr )
{
	if( skt <= 0 )
	{
		LogMsg( LOG_ERROR, "VxSktStatMgr::sktConnected2 invalid socket %d", skt );
		return;
	}

	m_SktStatMutex.lock();
	if( m_SktStatList.find( skt ) == m_SktStatList.end() )
	{
		VxSktStatRecord sktStat( skt, ipAddr );
		m_SktStatList[ skt ] = sktStat;
	}
	//else
	//{
	//	LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktConnected2 connected on existing socket %d", skt );
	//}

	m_SktStatMutex.unlock();
	LogModule( eLogConnect, LOG_INFO, "VxSktStatMgr::sktConnected2 skt handle %d connected cnt %zu", skt, m_SktStatList.size() );
}

//============================================================================
void VxSktStatMgr::sktConnected4( SOCKET skt, std::string ipAddr, ESktType sktType, EConnectReason connectReason )
{
	if( skt <= 0 )
	{
		LogMsg( LOG_ERROR, "VxSktStatMgr::%s invalid skt handle %d", __func__, skt );
		return;
	}

	if( ipAddr.empty() )
	{
		LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::%s skt handle %d ipAddr empty", __func__, skt );
		return;
	}

	m_SktStatMutex.lock();
	if( m_SktStatList.find( skt ) == m_SktStatList.end() )
	{
		VxSktStatRecord sktStat( skt, ipAddr, sktType, connectReason );
		m_SktStatList[ skt ] = sktStat;
	}
	//else
	//{
	//	LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktConnected3 connected on existing socket %d", skt );
	//}

	m_SktStatMutex.unlock();
	LogModule( eLogConnect, LOG_INFO, "VxSktStatMgr::sktConnected3 skt handle %d connected cnt %zu", skt, m_SktStatList.size() );
}

//============================================================================
void VxSktStatMgr::sktSetRemoteAddr( SOCKET skt, std::string ipAddr )
{
	if( skt <= 0 || ipAddr.empty() )
	{
		LogMsg( LOG_ERROR, "VxSktStatMgr::sktSetRemoteAddr invalid socket %d or empty ipAdd", skt );
		return;
	}

	m_SktStatMutex.lock();
	auto iter = m_SktStatList.find( skt );
	if( iter != m_SktStatList.end() )
	{
		iter->second.setIpAddr( ipAddr );
	}
	//else
	//{
	//	LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktSetRemoteAddr failed to find socket %d for ip %s", skt, ipAddr.c_str() );
	//}

	m_SktStatMutex.unlock();
}

//============================================================================
void VxSktStatMgr::sktSetType( SOCKET skt, ESktType sktType )
{
	if( skt <= 0 )
	{
		LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktSetType invalid socket %d", skt );
		return;
	}

	m_SktStatMutex.lock();
	auto iter = m_SktStatList.find( skt );
	if( iter != m_SktStatList.end() )
	{
		iter->second.setSktType( sktType );
	}
	//else
	//{
	//	LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktSetType failed to find socket %d", skt );
	//}

	m_SktStatMutex.unlock();
}

//============================================================================
void VxSktStatMgr::sktClosed( SOCKET skt )
{
	if( skt <= 0 )
	{
		LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktClosed invalid socket %d", skt );
		return;
	}

	m_SktStatMutex.lock();
	auto iter = m_SktStatList.find( skt );
	if( iter != m_SktStatList.end() )
	{
		m_SktStatList.erase( iter );
	}
	//else
	//{
	//	LogModule( eLogConnect, LOG_ERROR, "VxSktStatMgr::sktClosed failed to find socket %d", skt );
	//}

	m_SktStatMutex.unlock();
	LogModule( eLogConnect, LOG_INFO, "VxSktStatMgr::sktClosed skt %d connected cnt %zu", skt, m_SktStatList.size() );
}

//============================================================================
bool VxSktStatMgr::isAddressConnected( std::string ipAddr )
{
	if( ipAddr.empty() )
	{
		LogMsg( LOG_ERROR, "VxSktStatMgr::isAddressConnected empty ipAddr" );
		return false;
	}

	bool isConnected{ false };
	m_SktStatMutex.lock();
	for( auto& sktPair : m_SktStatList )
	{
		if( ipAddr == sktPair.second.getIpAddr() )
		{
			isConnected = true;
			break;
		}
	}

	m_SktStatMutex.unlock();
	return isConnected;
}
