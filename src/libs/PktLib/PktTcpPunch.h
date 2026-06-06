#ifndef PKT_TCP_PUNCH_H
#define PKT_TCP_PUNCH_H

//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "VxConnectInfo.h"

#pragma pack(push)
#pragma pack(1)

class PktTcpPunch : public VxPktHdr
{
public:
	PktTcpPunch();

	VxConnectInfo				m_ConnectInfo;

private:
    uint32_t					m_u32Res1;
    uint32_t					m_u32Res2;
    uint32_t					m_u32Res3;
    uint32_t					m_u32Res4;
    uint32_t					m_u32Res5;
    uint32_t					m_u32Res6;
};

#pragma pack(pop)

#endif // PKT_TCP_PUNCH_H
