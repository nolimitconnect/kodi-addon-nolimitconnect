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

#define PKT_CHAT_MAX_MSG_LEN			255
#define PKT_CHAT_MAX_IMAGES				20
#define PKT_CHAT_FLAG_IS_ASCII			0x0001
#define PKT_CHAT_FLAG_IS_CHOPPED		0x0002

#pragma pack(push)
#pragma pack(1)
class PktChatReq : public VxPktHdr
{
public:
	PktChatReq();

	char *						getDataPayload( void )				{ return m_au8Additional; }

	int							emptyLen( void );
	int							addImages( int iImageCnt, uint16_t * pu16Images );
	void						addMsg( const char* pMsg );

private:
	//=== vars ===//
	uint16_t					m_u16State{ 1 };
	uint16_t					m_u16Flags{ PKT_CHAT_FLAG_IS_ASCII };
	uint16_t					m_u16ImageCnt{ 0 };
	uint16_t					m_u16Res{ 0 };
	char						m_au8Additional[ PKT_CHAT_MAX_IMAGES * sizeof(uint16_t) * 2  + PKT_CHAT_MAX_MSG_LEN + 27 ];
};

#pragma pack(pop)




