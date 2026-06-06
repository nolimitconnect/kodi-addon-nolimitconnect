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

#include <CoreLib/VxValuePair.h>

#include <string>
#include <vector>

class VxUrl;

class VxHttpRequest
{
	std::string		m_strMethod;
	std::string		m_strRequestingHost;
	std::string		m_strHttpVersion;
	std::string		m_strUri;
	int				m_iPort;
	bool			m_bKeepConnectionAlive;

public:
	static const char* HTTP_GET;
	static const char* HTTP_POST;
	static const char* HTTP_HEAD;
	static const char* HTTP_SUBSCRIBE;
	static const char* HTTP_UNSUBSCRIBE;
	static const char* HTTP_NOTIFY;
	static const char* HTTP_CRLF;

	VxHttpRequest();
	VxHttpRequest( VxUrl& oUrl );

	void			setUrl( VxUrl& oUrl );

	void			setMethod( const char* pValue );
	const char*	getMethod( void );
	void			setRequestHost( const char* pHost );
	const char*	getRequestHost( void );
	void			setRequestPort( int iPort );
	int				getRequestPort( void );
	void			setURI( const char* pValue, bool useRelativeUrl = false );
	const char*	getURI( void );
	void			setHttpVersion1_0( void );
	void			setHttpVersion1_1( void );
	const char*	getHttpVersion( void );
	void			setKeepConnectionAlive( bool bKeepAlive );
	bool			isKeepAlive( void );

	const char*	buildGetHeader( std::string & strRetHeader );
	const char*	buildHost( std::string &headerBuf );
	const char*	buildUpnpSoapPost(	std::string &headerBuf, 
										const char* pSoapAction, 
										unsigned int contentLen );

	bool			isMethod(const char* pValue );
	bool			isGetRequest( void );
	bool			isPostRequest( void );
	bool			isHeadRequest( void );
	bool			isSubscribeRequest( void );
	bool			isUnsubscribeRequest( void );
	bool			isNotifyRequest( void );

	std::vector<VxValuePair> * getParameterList( std::vector<VxValuePair>& paramList );

	const char*getParameterValue(const char*name, std::string & strRetParamValue );

private:
	void initDefaults( void );
};


