//============================================================================
// Copyright (C) 2010-2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <NlcDependLibrariesConfig.h>
#include <CoreLib/StdMinMaxForWindows.h>

#include "PktsPluginOffer.h"

#include <CoreLib/VxDebug.h>

#include <string.h>
#include <string>

//============================================================================
PktPluginOfferReq::PktPluginOfferReq()
{
	setPktType( PKT_TYPE_PLUGIN_OFFER_REQ );
	setPktLength( sizeof( PktPluginOfferReq ) );

	vx_assert( 0 == (getPktLength() & 0x0f) );
}

//============================================================================
void PktPluginOfferReq::calcPktLen( void )
{
	uint16_t pktLen = (uint16_t)sizeof( PktPluginOfferReq ) - sizeof( PktBlobEntry );
	pktLen += getBlobEntry().getTotalBlobLen();
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen ) );

	vx_assert( 0 == (getPktLength() & 0x0f) );
}

//============================================================================
PktPluginOfferReply::PktPluginOfferReply()
{
	setPktType(  PKT_TYPE_PLUGIN_OFFER_REPLY );
	setPktLength(  sizeof( PktPluginOfferReply ) );

	vx_assert( 0 == ( getPktLength() & 0x0f ) );
}

//============================================================================
void PktPluginOfferReply::calcPktLen( void )
{
	uint16_t pktLen = (uint16_t)sizeof( PktPluginOfferReply ) - sizeof( PktBlobEntry );
	pktLen += getBlobEntry().getTotalBlobLen();
	setPktLength( ROUND_TO_16BYTE_BOUNDRY( pktLen ) );

	vx_assert( 0 == (getPktLength() & 0x0f) );
}

