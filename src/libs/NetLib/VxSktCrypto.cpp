//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktCrypto.h"
#include "VxSktBase.h"
#include "VxSktConnectSimple.h"

#include <PktLib/VxConnectId.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxGlobals.h>

//=================================================================
// Key generation methods
//=================================================================
static unsigned char g_au8RandomData[ 256 ] =
{
	#include "VxSktRandomData.h"
};

//============================================================================
unsigned char * GetVxSktRandData( int iRandDataIdx )
{
	return &g_au8RandomData[ iRandDataIdx ];
}

//============================================================================
//! generate key from net identity and connection data and place int sockets m_RxKey and initialize its crypto
bool GenerateRxConnectionKey(	std::shared_ptr<VxSktBase>&		sktBase,			
								VxConnectId *					poConnectId,		
								const char*						networkName )
								
{
	bool result{ false };
	sktBase->lockCryptoAccess();
	if( false == sktBase->m_RxKey.isKeySet() )
	{
		result = GenerateConnectionKey( &sktBase->m_RxKey, poConnectId, sktBase->getCryptoKeyPort(), networkName );
		if( result )
		{
			sktBase->m_RxCrypto.importKey( &sktBase->m_RxKey );
		}
		else
		{
            LogMsg( LOG_ERROR, "%s FAILED skt %d id %d port %d online id %s", __func__, sktBase->getSktHandle(), sktBase->getSktNumber(),
					sktBase->getCryptoKeyPort(), poConnectId->getOnlineId().toOnlineIdString().c_str() );
		}

		vx_assert( result );
	}

	sktBase->unlockCryptoAccess();
	return result;
}

//============================================================================
//! generate key from net identity and connection data and place int sockets m_TxKey and initialize its crypto
bool GenerateTxConnectionKey(	std::shared_ptr<VxSktBase>&		sktBase,			
								VxConnectId *					poConnectId,		
								const char*						networkName )
{
	bool result{ false };
	sktBase->lockCryptoAccess();
	if( false == sktBase->m_TxKey.isKeySet() )
	{
		result = GenerateConnectionKey(	 &sktBase->m_TxKey, poConnectId, sktBase->getCryptoKeyPort(), networkName );
		if( result )
		{
            // LogMsg( LOG_VERBOSE, "GenerateTxConnectionKey %s skt %d id %d", sktBase->m_TxKey.describeKey().c_str(), sktBase->getSktHandle(), sktBase->getSktNumber() );
			sktBase->m_TxCrypto.importKey( &sktBase->m_TxKey );
		}
		
		vx_assert( result );
	}

	sktBase->unlockCryptoAccess();
	return result;
}

//============================================================================
//! generate key from net identity and connection data and place int sockets m_RxKey and initialize its crypto
bool GenerateTxConnectionKey(  std::shared_ptr<VxSktBase>&	sktBase,
                                std::string					ipAddr,
                                uint16_t					port, 
                                VxGUID						onlineId,
                                std::string					networkName )
{
	bool result{ false };
    sktBase->lockCryptoAccess();
    std::string strNetworkName = networkName;
    if( false == sktBase->m_TxKey.isKeySet() )
    {
        result = GenerateConnectionKey(	&sktBase->m_TxKey, ipAddr, port, onlineId, sktBase->getCryptoKeyPort(), strNetworkName );
        if (result )
        {
            // LogMsg( LOG_VERBOSE, "GenerateTxConnectionKey %s skt %d id %d", sktBase->m_TxKey.describeKey().c_str(), sktBase->getSktHandle(), sktBase->getSktNumber() );
            sktBase->m_TxCrypto.importKey( &sktBase->m_TxKey );
        }
		 
        vx_assert( result );
    }

    sktBase->unlockCryptoAccess();
    return result;
}
