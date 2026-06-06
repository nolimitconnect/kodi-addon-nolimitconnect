#pragma once
//============================================================================
// Copyright (C) 2016 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"

#pragma pack(push)
#pragma pack(1)

class PktPingReq : public VxPktHdr
{
public:
	PktPingReq();

	void						setTimestamp( uint64_t timeStamp );
	uint64_t					getTimestamp( void );

private:
	uint64_t					m_Timestamp{ 0 };
};

class PktPingReply : public VxPktHdr
{
public:
	PktPingReply();

	void						setTimestamp( uint64_t timeStamp );
	uint64_t					getTimestamp( void );

private:
	uint64_t					m_Timestamp{ 0 };
};

#pragma pack(pop)

