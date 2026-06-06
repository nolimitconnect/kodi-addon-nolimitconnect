#ifndef PKT_VOICE_REPLY_H
#define PKT_VOICE_REPLY_H

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

#pragma pack(push)
#pragma pack(1)
class PktVoiceReply : public VxPktHdr
{
public:
	PktVoiceReply();

private:
	//=== vars ===//
    uint16_t					m_u16Res1;
    uint32_t					m_u32TimeMs;
    uint32_t					m_u32Res2;
    uint32_t					m_u32Res3;
    uint32_t					m_u32Res4;
};

#pragma pack(pop)

#endif // PKT_VOICE_REPLY_H
