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

#include "PluginSessionBase.h"

class P2PSession : public PluginSessionBase
{
public:
	P2PSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	P2PSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	virtual ~P2PSession();

	void						setOutstandingAckCnt( int cnt )				{ m_iOutstandingAckCnt = cnt; }
	int							getOutstandingAckCnt( void )				{ return m_iOutstandingAckCnt; }
	void						incrementOutstandingAckCnt( void )			{ m_iOutstandingAckCnt++; }
	void						decrementOutstandingAckCnt( void )			{ if( m_iOutstandingAckCnt ) m_iOutstandingAckCnt--; }

	void						setIsSendingPkts( bool isSending )			{ m_bSendingPkts = isSending; }
	int							getIsSendingPkts( void )					{ return m_bSendingPkts; }

	void						setVideoCastPkt( VxPktHdr* pktHdr )			{ m_VideoCastPkt = pktHdr; }
	VxPktHdr*					getVideoCastPkt( void )						{ return m_VideoCastPkt; }

protected:
	//=== vars ===//
	std::vector<VxPktHdr*>		m_aoPkts;

	int							m_iOutstandingAckCnt;
	bool						m_bSendingPkts;
	VxPktHdr*					m_VideoCastPkt;
};
