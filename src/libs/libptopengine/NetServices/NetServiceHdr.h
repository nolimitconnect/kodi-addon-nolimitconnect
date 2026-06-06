#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetServiceDefs.h"

#include <CoreLib/VxGUID.h>
class VxNetIdent;
class NetServiceHdr
{
public:
    NetServiceHdr() = default;

	ENetCmdError                getError() { return m_CmdError; }

	std::string					m_ChallengeHash;
    ENetCmdType					m_NetCmdType{ eNetCmdUnknown };
    int							m_CmdVersion{ 0 };
	ENetCmdError				m_CmdError{ eNetCmdErrorUnknown };
	int							m_TotalDataLen{ 0 };
	int							m_ContentDataLen{ 0 };
	VxGUID					    m_OnlineId;
	VxNetIdent*					m_Ident{ nullptr };
	int							m_SktDataUsed{ 0 };
};

