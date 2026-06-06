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

#include "VxPktHdr.h"
#include <CoreLib/IsBigEndianCpu.h>

#define PUSH_TO_TALK_PACKET_MAX_COMPRESSED_LEN 1280

#pragma pack(push)
#pragma pack(1)
class PktPushToTalkReq : public VxPktHdr
{
public:
	PktPushToTalkReq();

private:
	//=== vars ===//
	uint32_t					m_u32Res1{ 0 };
	uint32_t					m_u32Res2{ 0 };
};

class PktPushToTalkReply : public VxPktHdr
{
public:
	PktPushToTalkReply();

private:
	//=== vars ===//
	uint32_t					m_u32Res1{ 0 };
	uint32_t					m_u32Res2{ 0 };
};

class PktPushToTalkStart : public VxPktHdr
{
public:
	PktPushToTalkStart();

private:
	//=== vars ===//
	uint32_t					m_u32Res1{ 0 };
	uint32_t					m_u32Res2{ 0 };
};

class PktPushToTalkStop : public VxPktHdr
{
public:
	PktPushToTalkStop();

private:
	//=== vars ===//
	uint32_t					m_u32Res1{ 0 };
	uint32_t					m_u32Res2{ 0 };
};

#pragma pack(pop)



