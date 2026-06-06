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

#include <CoreLib/ISktStatCallbackInterface.h>
#include "VxSktStatRecord.h"

#include <CoreLib/VxMutex.h>
#include <PktLib/VxCommon.h>

#include <vector>
#include <map>

class VxSktStatMgr : public ISktStatCallbackInterface
{
public:
	VxSktStatMgr();
	virtual ~VxSktStatMgr();

	virtual void                sktConnected( SOCKET skt ) override;
	virtual void                sktConnected2( SOCKET skt, std::string ipAddr ) override;
	virtual void                sktConnected4( SOCKET skt, std::string ipAddr, ESktType sktType, EConnectReason connectReason ) override;
	virtual void                sktSetRemoteAddr( SOCKET skt, std::string ipAddr ) override;
	virtual void                sktSetType( SOCKET skt, ESktType sktType ) override;
	virtual void                sktClosed( SOCKET skt ) override;

	bool						isAddressConnected( std::string ipAddr );

	void						getSktStatRecords( std::vector<VxSktStatRecord>& retSktStatList );


	std::map<SOCKET,VxSktStatRecord> m_SktStatList;
	VxMutex						m_SktStatMutex;
};

