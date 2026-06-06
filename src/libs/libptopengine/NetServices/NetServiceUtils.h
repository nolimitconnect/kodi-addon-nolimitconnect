#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "NetServiceDefs.h"

#include <Network/NetworkDefs.h>

class VxSktBase;
class VxSktConnectSimple;
class VxGUID;
class P2PEngine;
class NetworkMgr;
class PktAnnounce;
class NetServiceHdr;
class VxKey;

class NetServiceUtils
{
public:
	NetServiceUtils( P2PEngine& engine );
    virtual ~NetServiceUtils() = default;

    NetworkMgr&					getNetworkMgr( void ) { return m_NetworkMgr; }

    std::string                 getNetworkKey( void );

    static ENetCmdType			netCmdStringToEnum( const char* netCmd );
	static const char*			netCmdEnumToString( ENetCmdType	eNetCmdType );

    static bool                 isRequestTypeNetCmd( ENetCmdType eNetCmdType );

    bool                        sendNetServiceRequest(  ENetCmdType         netCmdRequestType, ///< which type of net service to request
                                                        VxSktConnectSimple* netServConn, 
                                                        std::string&        netCmd,
                                                        int                 txDataTimeout );

    bool						rxNetServiceCmd(	ENetCmdType         expectedRxNetCmd, ///< which type of net service response to recieve
                                                    VxSktConnectSimple* netServConn, 
													char *				rxBuf, 
													int					rxBufLen, 
													NetServiceHdr&		netServiceHdr, 
													int					rxHdrTimeout = NETSERVICE_RX_URL_HDR_TIMEOUT, 
													int					rxDataTimeout = NETSERVICE_RX_DATA_TIMEOUT );

	EPluginType					parseHttpNetServiceUrl( std::shared_ptr<VxSktBase>& sktBase, NetServiceHdr& netServiceHdr );

    static EPluginType          parseHttpNetService( char* dataBuf, int dataLen, NetServiceHdr& netServiceHdr, std::string& netCmdContent );
	static EPluginType			parseHttpNetServiceHdr( char * dataBuf, int dataLen, NetServiceHdr& netServiceHdr );
	bool					    getNetServiceUrlContent( std::string& netServiceUrl, std::string& retFromClientContent );

	int							getIndexOfCrLfCrLf( std::shared_ptr<VxSktBase>& sktBase );

    bool					    buildCmd( std::string& retCmd, std::shared_ptr<VxSktBase>& sktBase, ENetCmdType netCmd, std::string& cmdContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );
    bool					    buildCmd( std::string& retCmd, VxSktConnectSimple* sktBase, ENetCmdType netCmd, std::string& cmdContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );

	int32_t						buildAndSendCmd( std::shared_ptr<VxSktBase>& sktBase, ENetCmdType netCmd, std::string& cmdContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );
    int32_t                       buildAndSendCmd( VxSktConnectSimple* sktBase, ENetCmdType netCmd, std::string& cmdContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );
    
    bool						buildIsMyPortOpenUrl( VxSktConnectSimple * netServConn, std::string& strHttpUrl, uint16_t u16Port );
    bool						buildQueryHostIdUrl( VxSktConnectSimple * netServConn, std::string& strNetCmdHttpUrl );
    bool						buildPingTestUrl( VxSktConnectSimple * netServConn, std::string& strNetCmdHttpUrl );

    bool 						buildNetCmd( VxSktConnectSimple * netServConn, std::string& retResult, ENetCmdType netCmd, std::string& strContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );
    bool 						buildNetCmd( uint16_t cryptoPort, std::string& retResult, ENetCmdType netCmd, std::string& strContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );
    static void					buildNetCmd( std::string& retResult, ENetCmdType netCmd, std::string& netServChallengeHash, std::string& strContent, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );

    // returns total length of data to send
    int							buildNetCmdHeader( std::string& retResult, ENetCmdType netCmd, std::string& netServChallengeHash, int contentLength, ENetCmdError errCode = eNetCmdErrorNone, int version = 1 );

    static void					generateNetServiceChallengeHash(	std::string&			strKey,
                                                                    std::shared_ptr<VxSktBase>&				skt,
                                                                    std::string             netKey);
    static void					generateNetServiceChallengeHash(	std::string&			strKey,
                                                                    VxSktConnectSimple *	skt,
                                                                    std::string             netKey);
    static void					generateNetServiceChallengeHash(	std::string&			strKey,		// set this key
                                                                    uint16_t				clientPort,
                                                                    std::string             netKey);
    static void					generateNetServiceCryptoKey(		VxKey&					key,
                                                                    uint16_t				clientPort,
                                                                    std::string             netKey);
    static bool                 generateNetPktCryptoPassword( std::string& retPwd, std::string netKey, uint16_t port, std::string ip );

	static bool					verifyAllDataArrivedOfNetServiceUrl( std::shared_ptr<VxSktBase>& sktBase );
	// returns content length or -1 if invalid url
	static int					getTotalLengthFromNetServiceUrl(  char * dataBuf, int dataLen );

	bool						decryptNetServiceContent( char * content, int contentDataLen, uint16_t clientPort );

	static bool					isValidHexString( const char* hexString, int dataLen );

    static bool                 isAllAscii( std::string& netCmd );

protected:
	static int					getNextPartOfUrl( char * buf, std::string& strValue );

	P2PEngine&					m_Engine;
	NetworkMgr&					m_NetworkMgr;
	PktAnnounce&				m_PktAnn;
};


