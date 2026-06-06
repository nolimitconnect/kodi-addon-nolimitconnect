//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktAnnList.h"
#include "PktAnnounce.h"

#include <memory.h>

//============================================================================
PktAnnList::PktAnnList()
{ 
	setPktLength( emptyLen() ); 
	setPktType( PKT_TYPE_ANN_LIST ); 
	m_u16Flags = 0;
	m_u16ListCnt = 0; 
	m_u16Reason = 0;
	m_u16Res1 = 0;
	m_u32Res2 = 0;
}

//============================================================================
int PktAnnList::emptyLen( void )
{  
	return sizeof( PktAnnList ) - MAX_PKT_ANN_LIST_LEN ; 
}

//============================================================================
void PktAnnList::calcPktLen( void )
{ 
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( getPktLength() ) ); 
}

//============================================================================
int PktAnnList::addAnn( VxNetIdent* poPktAnn )
{
	if( sizeof( VxNetIdent ) * m_u16ListCnt > MAX_PKT_ANN_LIST_LEN )
	{
		return -1;
	}

	int iOffs = getPktLength() - emptyLen();
	*(( uint16_t *)&m_au8List[ iOffs ]) = ntohs( (uint16_t)sizeof( VxNetIdent ) );
	iOffs += 2;
	memcpy( &m_au8List[ iOffs ], poPktAnn, sizeof( VxNetIdent ) );
	m_u16ListCnt++;
	setPktLength( getPktLength() + sizeof( uint16_t ) + sizeof( VxNetIdent ) );
	return 0;
}
