#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>

#include "AssetBaseXferSession.h"

class AssetBaseTxSession : public AssetBaseXferSession
{
public:
	AssetBaseTxSession( P2PEngine& engine );
	AssetBaseTxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	AssetBaseTxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );

	void						reset( void );
	void						cancelUpload( VxGUID& fileInstance );
	void						setQuePosition( int quePos )				{ m_QuePosition = quePos; }
	int							getQuePosition( void )						{ return m_QuePosition; }

	//=== vars ===//
	int							m_iOutstandingAckCnt{ 0 }; // how many receive acks are outstanding
	bool						m_bSendingPkts{ false };
	int							m_QuePosition{ 0 };
	std::string					m_strOfferFile;
	std::string					m_strViewDirectory;
};
