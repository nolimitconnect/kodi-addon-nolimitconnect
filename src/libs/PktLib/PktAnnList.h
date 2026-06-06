#pragma once
//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

class VxNetIdent;

#include "VxPktHdr.h"

#pragma pack(push)
#pragma pack(1)
#define MAX_PKT_ANN_LIST_LEN 4080

class PktAnnList : public VxPktHdr
{
public:
	PktAnnList();

	int							emptyLen( void );
	void						calcPktLen( void );
	int						    addAnn( VxNetIdent* pgPktAnn );

private:
	//=== vars ===//
	uint16_t					m_u16ListCnt;
	uint16_t					m_u16Flags;
	uint32_t					m_u16Reason;
	uint32_t					m_u16Res1;
	uint32_t					m_u32Res2;
	uint8_t						m_au8List[ MAX_PKT_ANN_LIST_LEN ];
};

#pragma pack(pop)
