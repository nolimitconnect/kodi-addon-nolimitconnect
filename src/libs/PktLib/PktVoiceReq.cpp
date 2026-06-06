//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "PktVoiceReq.h"

//============================================================================
PktVoiceReq::PktVoiceReq()
{
	setPktType( PKT_TYPE_VOICE_REQ );
}

//============================================================================
void PktVoiceReq::calcPktLen( void )
{
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( (sizeof( PktVoiceReq ) - ( sizeof( m_CompressedData ) ) + (getCompressedDataLen()) ) ) );
}

