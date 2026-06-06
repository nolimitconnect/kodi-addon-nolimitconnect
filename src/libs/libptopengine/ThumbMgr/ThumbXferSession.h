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

#include "ThumbInfo.h"
#include <AssetBase/AssetBaseXferSession.h>

#include <NetLib/VxFileXferInfo.h>

#include <vector>

class VxSktBase;
class VxNetIdent;
class P2PEngine;

class ThumbXferSession : public AssetBaseXferSession
{
public:
	ThumbXferSession( P2PEngine& engine );
    ThumbXferSession( P2PEngine& engine, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId );
    ThumbXferSession( P2PEngine& engine, VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId  );
	virtual ~ThumbXferSession();

	void						setThumbInfo( ThumbInfo& assetInfo )	        { m_ThumbInfo = assetInfo; }
	ThumbInfo&				    getAssetInfo( void )						    { return m_ThumbInfo; }

	void						setSkt( std::shared_ptr<VxSktBase>& skt )	{ m_Skt = skt; }
	std::shared_ptr<VxSktBase>&	getSkt( void )								{ return m_Skt; }

	void						setLclSessionId( VxGUID& lclId )			{ m_FileXferInfo.setLclSessionId( lclId ); }
	VxGUID&						getLclSessionId( void )						{ return m_FileXferInfo.getLclSessionId(); }
	void						setRmtSessionId( VxGUID& rmtId )			{ m_FileXferInfo.setRmtSessionId( rmtId ); }
	VxGUID&						getRmtSessionId( void )						{ return m_FileXferInfo.getRmtSessionId(); }

	void						setFileHashId( uint8_t * fileHashId )		{ m_FileXferInfo.setFileHashId( fileHashId ); }
	void						setFileHashId( VxSha1Hash& fileHashId )		{ m_FileXferInfo.setFileHashId( fileHashId ); }
	VxSha1Hash&					getFileHashId( void )						{ return m_FileXferInfo.getFileHashId(); }

	VxFileXferInfo&				getXferInfo( void )							{ return m_FileXferInfo; }
	void						setXferDirection( EXferDirection dir )		{ m_FileXferInfo.setXferDirection( dir ); }
	EXferDirection				getXferDirection( void )					{ return m_FileXferInfo.getXferDirection(); }

	void						reset( void );
	bool						isXferingFile( void );

	void						setErrorCode( int32_t error )						{ if( error ) m_Error = error; } 
	int32_t						getErrorCode( void )							{ return m_Error; }
	void						clearErrorCode( void )							{ m_Error = 0; }

	void						setThumbStateSendBegin( void );
	void						setThumbStateSendProgress( int progress );
	void						setThumbStateSendFail( void );
	void						setThumbStateSendCanceled( void );
	void						setThumbStateSendSuccess( void );

	//=== vars ===//
protected:
	ThumbInfo				    m_ThumbInfo;

private:
	void						initLclSessionId( void );
};
