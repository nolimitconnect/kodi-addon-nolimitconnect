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

#include "FileToXfer.h"

#include <NetLib/VxFileXferInfo.h>

#include <memory>
#include <vector>

class VxSktBase;

class FileShareXferSession
{
public:
	FileShareXferSession();
	FileShareXferSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	FileShareXferSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );

    void						setIsStream( bool isStreaming )				{ m_FileXferInfo.setIsStream( isStreaming ); }
	bool 						isStream( void )							{ return m_FileXferInfo.isStream(); }

	void						setSendToId( VxGUID sendToId )				{ m_SendToId = sendToId; }
	VxGUID&						getSendToId( void )							{ return m_SendToId; }

	void						setSkt( std::shared_ptr<VxSktBase>& skt )	{ m_Skt = skt; }
	std::shared_ptr<VxSktBase>&	getSkt( void )								{ return m_Skt; }

	void						setLclSessionId( VxGUID& lclId )			{ m_FileXferInfo.setLclSessionId( lclId ); }
	VxGUID&						getLclSessionId( void )						{ return m_FileXferInfo.getLclSessionId(); }
	void						setRmtSessionId( VxGUID& rmtId )			{ m_FileXferInfo.setRmtSessionId( rmtId ); }
	VxGUID&						getRmtSessionId( void )						{ return m_FileXferInfo.getRmtSessionId(); }

	void						setAssetId( VxGUID& assetId )				{ m_FileXferInfo.setAssetId( assetId ); }
	VxGUID&						getAssetId( void )							{ return m_FileXferInfo.getAssetId(); }

	void						setFileHashId( uint8_t * fileHashId )		{ m_FileXferInfo.setFileHashId( fileHashId ); }
	void						setFileHashId( VxSha1Hash& fileHashId )		{ m_FileXferInfo.setFileHashId( fileHashId ); }
	VxSha1Hash&					getFileHashId( void )						{ return m_FileXferInfo.getFileHashId(); }

	VxFileXferInfo&				getXferInfo( void )							{ return m_FileXferInfo; }
	void						setXferDirection( EXferDirection dir )		{ m_FileXferInfo.setXferDirection( dir ); }
	EXferDirection				getXferDirection( void )					{ return m_FileXferInfo.getXferDirection(); }

	void						reset( void );
	bool						isXferingFile( void );

	void						setErrorCode( int32_t error )					{ m_Error = error; } 
	int32_t						getErrorCode( void )						{ return m_Error; }
	void						clearErrorCode( void )						{ m_Error = 0; }

	bool						offerAccepted( VxGUID& lclSessionId );
	bool						setupNextFile( void );
	bool						fillXferFile( FileToXfer& fileToXfer );

	//=== vars ===//
	std::vector<FileToXfer>		m_FilesToXferList;		// list of files to send
	std::vector<FileToXfer>		m_FilesXferedList;		// list of files sent

protected:
	VxFileXferInfo				m_FileXferInfo;		// file being transmitted
	int							m_iPercentComplete{ 0 };
	std::shared_ptr<VxSktBase>	m_Skt;
	VxGUID						m_SendToId;
	uint32_t					m_Error{ 0 };

private:
	void						initLclSessionId( void );
};
