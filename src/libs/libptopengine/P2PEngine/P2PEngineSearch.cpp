//============================================================================
// Copyright (C) 2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <P2PEngine/P2PEngine.h>
#include <Search/RcScan.h>
#include <BigListLib/BigListInfo.h>
#include <BigListLib/BigListMgr.h>
#include <CoreLib/VxDebug.h>
#include <PktLib/VxCommon.h>

//============================================================================
void P2PEngine::onPktScanReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	VxNetIdent* netIdent = m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	if( netIdent )
	{
		m_RcScan.onPktScanReq( netIdent, sktBase, (PktScanReq *)pktHdr );
		return;
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::onPktScanReq unknown ident 0x%llX 0x%llX\n", 
			pktHdr->getSrcOnlineId().getVxGUIDHiPart(),
			pktHdr->getSrcOnlineId().getVxGUIDLoPart() );
	}
}

//============================================================================
void P2PEngine::onPktScanReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	VxNetIdent* netIdent = m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	if( netIdent )
	{
		m_RcScan.onPktScanReply( netIdent, sktBase, (PktScanReply *)pktHdr );
		return;
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::onPktScanReply unknown ident 0x%llX 0x%llX\n", 
			pktHdr->getSrcOnlineId().getVxGUIDHiPart(),
			pktHdr->getSrcOnlineId().getVxGUIDLoPart() );
	}
}

//============================================================================
void P2PEngine::onPktFindFileReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	VxNetIdent* netIdent = m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	if( netIdent )
	{
		m_RcScan.onPktFindFileReq( netIdent, sktBase, (PktFindFileReq *)pktHdr );
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::onPktFindFileReq unknown ident 0x%llX 0x%llX\n", 
			pktHdr->getSrcOnlineId().getVxGUIDHiPart(),
			pktHdr->getSrcOnlineId().getVxGUIDLoPart() );
	}
}

//============================================================================
void P2PEngine::onPktFindFileReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr )
{
	VxNetIdent* netIdent = m_BigListMgr.findBigListInfo( pktHdr->getSrcOnlineId() );
	if( netIdent )
	{
		//m_RcScan.onPktFindFileReply( netIdent, sktBase, (PktFindFileReply *)pktHdr );
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::onPktFindFileReply unknown ident 0x%llX 0x%llX\n", 
			pktHdr->getSrcOnlineId().getVxGUIDHiPart(),
			pktHdr->getSrcOnlineId().getVxGUIDLoPart() );
	}
}

