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

#include "P2PSession.h"
#include "FileToXfer.h"

#include <NetLib/VxFileXferInfo.h>

class VxSktBase;
class VxPktHdr;
class PluginSessionFileXfer;
class FileXferDirectories;
class PluginBase;

class FileXferPluginSession : public P2PSession
{
public:
	FileXferPluginSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	FileXferPluginSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID sendToId, EPluginType pluginType );
	virtual ~FileXferPluginSession();

	//! return true if file is already in que to be sent or has been sent
	bool						isFileQuedOrSent( const char* pFileName );

	VxFileXferInfo&				getRxXferInfo( void )							{ return m_RxFileInfo; }
	VxFileXferInfo&				getTxXferInfo( void )							{ return m_TxFileInfo; }

	void						cancelUpload( PluginBase& pluginBase, VxGUID& lclSessionId );
	void						cancelDnload( PluginBase& pluginBase, VxGUID& lclSessionId );

	void						setErrorCode( int32_t error )					{ if( error ) m_Error = error; } 
	int32_t						getErrorCode( void )						{ return m_Error; }
	void						clearErrorCode( void )						{ m_Error = 0; }


	VxFileXferInfo				m_TxFileInfo;			// file being transmitted
	VxFileXferInfo				m_RxFileInfo;			// file being received

	std::vector<FileToXfer>		m_astrFilesToSend;		// list of files to send
	std::vector<FileToXfer>		m_astrFilesSent;		// list of files sent
	std::vector<FileToXfer>		m_astrFilesReceived;		// list of files sent
	uint32_t					m_Error;
};
