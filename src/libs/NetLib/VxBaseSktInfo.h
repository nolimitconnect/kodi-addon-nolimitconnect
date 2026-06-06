#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxFileXferInfo.h"

#include <vector>
#include <memory>

class VxSktBase;
class VxNetIdent;

class VxBaseSktInfo
{
public:
	// base class for per connection info.. contains
	// whats needed for file transfers

	VxBaseSktInfo();
	virtual ~VxBaseSktInfo();

	//! return true if file is already in que to be sent or has been sent
	bool isFileQuedOrSent( const char* pFileName );
	//! que a file to send
	void queFileToSend( const char* pLclFileName, const char* pRmtFileName );

protected:
	std::shared_ptr<VxSktBase>	m_Skt;					// socket
	uint32_t					m_u32PeerIp{0};			// peer's ip in binary form
	uint32_t					m_u32LocalIp{0};		// our ip in binary form
	uint32_t					m_u32LastRxTime{0};		// last time received a packet
	//=== identity vars ===//
	VxNetIdent*					m_Ident;				// network identity
	std::string					m_csUserName;			// users login name

	//=== file tx/rx vars ===//
	VxFileXferInfo				m_TxFileInfo;			// file being transmitted
	VxFileXferInfo				m_RxFileInfo;			// file being received
	std::vector<std::string>	m_astrFilesToSend;		// list of files to send
	std::vector<std::string>	m_astrFilesSent;		// list of files sent
	std::vector<std::string>	m_astrFilesReceived;	// list of files received

};
