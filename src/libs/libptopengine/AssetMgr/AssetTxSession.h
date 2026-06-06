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

#include "AssetXferSession.h"

class AssetTxSession : public AssetXferSession
{
public:
	AssetTxSession( P2PEngine& engine );
	AssetTxSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );
	AssetTxSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );

	void						reset( void );
	void						cancelUpload( VxGUID& fileInstance );
	void						setQuePosition( int quePos )				{ m_QuePosition = quePos; }
	int							getQuePosition( void )						{ return m_QuePosition; }

	//=== vars ===//
	int							m_iOutstandingAckCnt; // how many receive acks are outstanding
	bool						m_bSendingPkts;
	bool						m_bViewingFileList;
	int							m_QuePosition;
	std::string					m_strOfferFile;
	std::string					m_strViewDirectory;
};
