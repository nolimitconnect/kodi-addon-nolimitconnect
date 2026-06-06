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

#include "AssetBaseInfo.h"

#include <NetLib/VxFileXferInfo.h>

#include <vector>
#include <memory>

class P2PEngine;
class VxNetIdent;
class VxSktBase;

class AssetBaseXferSession
{
public:
	AssetBaseXferSession( P2PEngine& engine );
	AssetBaseXferSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	AssetBaseXferSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	virtual ~AssetBaseXferSession();

	void						setSendToId( VxGUID sendToId )					{ m_SendToId = sendToId; }
	VxGUID&						getSendToId( void )							    { return m_SendToId; }

	void						setAssetBaseInfo( AssetBaseInfo& assetInfo )	{ m_AssetBaseInfo = assetInfo; }
	AssetBaseInfo&				getAssetBaseInfo( void )						{ return m_AssetBaseInfo; }

	void						setIsStream( bool isStream )					{ m_IsStream = isStream; }
	bool						getIsStream( void )							    { return m_IsStream; }

	void						setSkt( std::shared_ptr<VxSktBase>& skt )		{ m_Skt = skt; }
	std::shared_ptr<VxSktBase>&	getSkt( void )								    { return m_Skt; }

	void						setLclSessionId( VxGUID& lclId )			    { m_FileXferInfo.setLclSessionId( lclId ); }
	VxGUID&						getLclSessionId( void )						    { return m_FileXferInfo.getLclSessionId(); }
	void						setRmtSessionId( VxGUID& rmtId )			    { m_FileXferInfo.setRmtSessionId( rmtId ); }
	VxGUID&						getRmtSessionId( void )						    { return m_FileXferInfo.getRmtSessionId(); }

	void						setFileHashId( uint8_t * fileHashId )		    { m_FileXferInfo.setFileHashId( fileHashId ); }
	void						setFileHashId( VxSha1Hash& fileHashId )		    { m_FileXferInfo.setFileHashId( fileHashId ); }
	VxSha1Hash&					getFileHashId( void )						    { return m_FileXferInfo.getFileHashId(); }

	VxFileXferInfo&				getXferInfo( void )							    { return m_FileXferInfo; }
    void						setXferDirection( enum EXferDirection dir )		{ m_FileXferInfo.setXferDirection( dir ); }
	EXferDirection				getXferDirection( void )					    { return m_FileXferInfo.getXferDirection(); }

	void						reset( void );
	bool						isXferingFile( void );

	void						setErrorCode( int32_t error )						{ if( error ) m_Error = error; } 
	int32_t						getErrorCode( void )							{ return m_Error; }
	void						clearErrorCode( void )							{ m_Error = 0; }

	void						setAssetBaseStateSendBegin( void );
	void						setAssetBaseStateSendProgress( int progress );
	void						setAssetBaseStateSendFail( void );
	void						setAssetBaseStateSendCanceled( void );
	void						setAssetBaseStateSendSuccess( void );

	//=== vars ===//
protected:
	P2PEngine&					m_Engine; 
	VxFileXferInfo				m_FileXferInfo;		// file being transmitted
    int							m_iPercentComplete{ 0 };
	std::shared_ptr<VxSktBase>	m_Skt;
	VxGUID						m_SendToId;
	uint32_t					m_Error{ 0 };
	AssetBaseInfo				m_AssetBaseInfo;
	bool						m_IsStream{ false };

private:
	void						initLclSessionId( void );
};
