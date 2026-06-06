//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"

#include <CoreLib/VxDebug.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <array>

//============================================================================
void P2PEngine::hackerOffense( enum EHackerLevel	hackerLevel,
                                enum EHackerReason   hackerReason,
                                InetAddress		ipAddr,					// ip address if identity not known
                                VxGUID          signatureGuid,		// users identity info ( may be null if not known then use ipAddress )
                                const char*     pMsg, ... )			// message about the offense
{
    std::array< char, 4096 > szBuffer;
    szBuffer.data()[ 0 ] = 0;
    if( pMsg )
    {
        va_list argList;
        va_start( argList, pMsg );
        vsnprintf( szBuffer.data(), szBuffer.size(), pMsg, argList);
        szBuffer[ szBuffer.size() - 1 ] = 0;
        va_end( argList );
    }

    std::string strIp = ipAddr.toString();
    LogModule( eLogHackers, LOG_SEVERE, "%s %s: ip %s %s", DescribeHackerLevel( hackerLevel ), DescribeHackerReason( hackerReason ), strIp.c_str(), szBuffer.data() );
    if( hackerLevel >= eHackerLevelMedium )
    {
        getPeerMgr().addHackOffense( hackerLevel, hackerReason, strIp, signatureGuid );
    }
}

//============================================================================
//! called if hacker offense is detected
void P2PEngine::hackerOffense( enum EHackerLevel	hackerLevel,
                                enum EHackerReason	hackerReason,
                                VxNetIdent*	    poContactIdent,			// users identity info ( may be null if not known then use ipAddress )                             
								InetAddress		IpAddr,					// ip address if identity not known
								const char*	    pMsg, ... )				// message about the offense
{
	std::array< char, 4096 > szBuffer;
    szBuffer.data()[0] = 0;
    if( pMsg )
    {
        va_list argList;
        va_start( argList, pMsg );
        vsnprintf( szBuffer.data(), szBuffer.size(), pMsg, argList);
        szBuffer[ szBuffer.size() - 1 ] = 0;
        va_end( argList );
    }

    InetAddress oIpAddr = IpAddr;
    if( poContactIdent )
    {
        oIpAddr = poContactIdent->getOnlineIpAddress();
    }

	std::string strIp = oIpAddr.toString();
    LogModule( eLogHackers, LOG_SEVERE, "%s %s: ip %s %s", DescribeHackerLevel( hackerLevel ), DescribeHackerReason( hackerReason ), strIp.c_str(), szBuffer.data() );
    if( hackerLevel >= eHackerLevelMedium )
    {
        VxGUID emptyGuid;
        getPeerMgr().addHackOffense( hackerLevel, hackerReason, strIp, poContactIdent ? poContactIdent->getMyOnlineId() : emptyGuid );
    }
}

//============================================================================
//! called if hacker offense is detected
void P2PEngine::hackerOffense( enum EHackerLevel	hackerLevel,
                                enum EHackerReason   hackerReason,
                                VxPktHdr*       pktHdr,		
                                std::shared_ptr<VxSktBase>&      sktBase,
                                const char*	    pMsg, ... )				// message about the offense
{
	std::array< char, 4096 > szBuffer;
    szBuffer.data()[0] = 0;
    if( pMsg )
    {
        va_list argList;
        va_start( argList, pMsg );
        vsnprintf( szBuffer.data(), szBuffer.size(), pMsg, argList);
        szBuffer[ szBuffer.size() - 1 ] = 0;
        va_end( argList );
    }

    LogModule( eLogHackers, LOG_SEVERE, "%s %s: %s", DescribeHackerLevel( hackerLevel ),  DescribeHackerReason( hackerReason ), szBuffer.data() );

    if( hackerLevel >= eHackerLevelMedium )
    {
        std::string ipAddr = sktBase->getRemoteIpAddress();
        getPeerMgr().addHackOffense( hackerLevel, hackerReason, ipAddr, pktHdr->getSrcOnlineId() );
    }
}
