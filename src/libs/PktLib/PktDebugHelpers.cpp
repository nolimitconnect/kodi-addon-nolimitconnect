//============================================================================
// Copyright (C) 2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktDebugHelpers.h"
#include "VxPktHdr.h"

#include <CoreLib/VxDebug.h>

void DumpPkt( VxPktHdr* pktHdr )
{
	LogMsg( LOG_INFO, "PktDump len %d type %d ver %d",
			pktHdr->getPktLength(),
			pktHdr->getPktType(),
			pktHdr->getPktVersionNum() );

	if( pktHdr->getPktLength() > sizeof( VxPktHdr ) )
	{
		unsigned char * data = (unsigned char *)pktHdr;
		data += sizeof( VxPktHdr );

        HexDump( LOG_INFO, data, pktHdr->getPktLength() - sizeof( VxPktHdr ), 0, (char *)"VxPkt" );
	}
}
