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

class RcScanPic
{
public:
	RcScanPic( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase, uint8_t * u8JpgData, uint32_t u32JpgDataLen )
		: m_Ident(netIdent)
		, m_Skt(sktBase)
		, m_u8JpgData(u8JpgData)
		, m_u32JpgDataLen(u32JpgDataLen)
	{
	}

	//=== vars ===//
	VxNetIdent*					m_Ident; 
	std::shared_ptr<VxSktBase>&	m_Skt;
	uint8_t *					m_u8JpgData;
	uint32_t					m_u32JpgDataLen;
};