#pragma once
//============================================================================
// Copyright (C) 2008 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxCrypto.h>
#include <CoreLib/VxSktUtil.h>

class VxSktConnectSimple
{
public:
	VxSktConnectSimple();
	virtual ~VxSktConnectSimple();

	SOCKET						getSktHandle( void )							{ return m_Socket; }
	int                         getSktNumber( void )							{ return m_SktNumber; }

	uint16_t					getRemotePort( void )							{ return m_RmtIp.getPort(); }
    std::string				    getRemoteIpAddress( void )                      { return m_RmtIp.toString(); }
	uint16_t					getLocalPort( void )							{ return m_LclIp.getPort(); }
    void				        setLocalIpAddress( const char* lclIp )			{ m_LclIp.setIp( lclIp ); }
	std::string					getLocalIpAddress( void );
    void				        setLocalIpAndPort( const char* lclIp, uint16_t port )			{ m_LclIp.setIpAndPort( lclIp, port ); }

	uint16_t					getCryptoKeyPort( void )						{ return m_RmtIp.getPort(); }

	virtual bool				isConnected( void );

    virtual SOCKET				connectTo( const char*	pIpOrUrl,						// remote ip or url
                                           uint16_t		u16Port,						// port to connect to
										   EIpAddrType  addrType = eIpAddrTypeUnknown,
                                           int			iTimeoutMilliSeconds = WEBSITE_CONNECT_TIMEOUT,
										   int32_t*		retErrorCode = nullptr );	

    virtual SOCKET				connectTo( const char* lclAdapterIp,					// local adapter ip
                                           const char*	pIpOrUrl,						// remote ip or url
                                           uint16_t		u16Port,						// port to connect to
										   EIpAddrType  addrType = eIpAddrTypeUnknown,
                                           int			iTimeoutMilliSeconds = WEBSITE_CONNECT_TIMEOUT,
										   int32_t*		retErrorCode = nullptr );	

	virtual bool				connectToWebsite(	const char*			pWebsiteUrl,
													std::string&		strHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
													std::string&		strFile,		// return file name.. images/me.png
													uint16_t&			u16Port,
													EIpAddrType			addrType = eIpAddrTypeUnknown,
													int					iConnectTimeoutMs = WEBSITE_CONNECT_TIMEOUT,
													int*				retErrorCode = nullptr );	

	virtual bool				connectToWebsite(	const char*			pWebsiteUrl,
													std::string&		strHost,		// return host name.. example http://www.mysite.com/index.htm returns www.mysite.com
													std::string&		strFile,		// return file name.. images/me.png
													uint16_t&			u16Port,
													std::string&		strResolveIpAddr,
													EIpAddrType			addrType = eIpAddrTypeUnknown,
													int					iConnectTimeoutMs = WEBSITE_CONNECT_TIMEOUT,
												    int*				retErrorCode = nullptr );	

	virtual int32_t				sendData(	const char*		pData,							// data to send
											int				iDataLen,						// length of data	
											int				timeoutMs = SKT_SEND_TIMEOUT );	// timeout attempt to send ( 0 = don't timeout )

	//! receive data.. if timeout is set then will keep trying till buffer is full or error or timeout expires
	virtual int32_t				recieveData( char* pRetDataBuf,					// data buffer to read into
											 int   iBufLen,						// length of data	
											 int*  iRetBytesReceived,				// number of bytes actually received
											 int   timeoutMs = SKT_RECEIVE_TIMEOUT );	// timeout attempt to received

	virtual void				closeSkt( int iInstance = 0 );

	void						dumpConnectionInfo( void );
	
	/// return true if transmit encryption key is set
	virtual bool				isTxEncryptionKeySet( void )        { return m_TxKey.isKeySet(); }
	/// return true if receive encryption key is set
	virtual bool				isRxEncryptionKeySet( void )        { return m_RxKey.isKeySet(); }
	void						setTxCryptoPassword( const char* data, int len );
	void						setRxCryptoPassword( const char* data, int len );
	bool						isTxCryptoKeySet( void );
	bool						isRxCryptoKeySet( void );

	bool						encryptAndSendTxData( const char* data, int len, int timeoutMs );
	/// decrypt data .. data is decrypted in place 
	bool						decryptReceiveData( char* data, int iDataLen );

	int32_t						setLastError( int32_t rc )			{ m_LastError = rc; return rc; }
	int32_t						getLastError( void )				{ return m_LastError; }

	//=== vars ===//
	int32_t						m_LastError{ 0 };
	SOCKET						m_Socket;				// handle to socket
	int							m_SktNumber;			// socket unique id
	bool						m_bIsConnected;			// return true if is connected

	InetAddrAndPort				m_LclIp;				// local ip address
	InetAddrAndPort				m_RmtIp;				// remote (peer) ip address

	VxKey						m_RxKey;				        // encryption key for receive
	VxCrypto					m_RxCrypto;			            // encryption object for receive
	VxKey						m_TxKey;				        // encryption key for transmit
	VxCrypto					m_TxCrypto;			            // encryption object for transmit
};

