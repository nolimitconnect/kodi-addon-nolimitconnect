//============================================================================
// Copyright (C) 2016 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RcScanMatchedConnection.h"

#include <Search/RcScan.h>


//============================================================================
RcScanMatchedConnection::RcScanMatchedConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase )
: m_Ident( netIdent )
, m_Skt( sktBase )
{
}

//============================================================================
RcScanMatchedConnection::RcScanMatchedConnection( const RcScanMatchedConnection& rhs )
{
	*this = rhs;
}

//============================================================================
RcScanMatchedConnection& RcScanMatchedConnection::operator=( const RcScanMatchedConnection& rhs ) 
{	
	if( this != &rhs )
	{
		m_Ident					= rhs.m_Ident;
		m_Skt					= rhs.m_Skt;
		m_ActionStartTimeMs		= rhs.m_ActionStartTimeMs;
		m_u8JpgData				= rhs.m_u8JpgData;
		m_u32JpgDataLen			= rhs.m_u32JpgDataLen;
		m_ActionHadError		= rhs.m_ActionHadError;
		m_ActionComplete		= rhs.m_ActionComplete;
	}

	return *this;
}

//============================================================================
void RcScanMatchedConnection::deleteResources( void )
{
	delete m_u8JpgData;
	m_u8JpgData = 0;
}
