#pragma once
//============================================================================
// Copyright (C) 2017 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/GroupieId.h>

class VxPtopUrl
{
public:
	VxPtopUrl() = default;
	VxPtopUrl( std::string& url );
	VxPtopUrl( const char* url );
    VxPtopUrl( const VxPtopUrl& rhs );

    VxPtopUrl&                  operator = ( const VxPtopUrl& rhs );
    bool                        operator == ( const VxPtopUrl& rhs ) const;

    bool						isValid( bool doesNotRequireOnlineId = false );
	bool						isHostIpValid( void );
	bool						isHostTypeValid( void )						{ return m_HostType != eHostTypeUnknown; }

	bool						isUrlIpv4( void );
	bool						isUrlIpv6( void );
	std::string					getHostUrl( void );
    GroupieId                   getHostGroupieId( void );

	void						setUrl( std::string url );
	void						setUrl( const char* url );
	std::string&				getUrl( void )							    { return m_Url; }

	std::string&				getProtocol( void )							{ return m_Protocol; }
    std::string&                getHost( void )								{ return m_Host; }
	uint16_t					getPort( void )								{ return m_Port; }
	VxGUID&						getOnlineId( void )							{ return m_OnlineId; }
	EHostType					getHostType( void )							{ return m_HostType; }

	std::string					stripHost( const std::string& url ) const; // remove suffix invite type if exists

	// if forceHostType then set group type even if url does not end with !
	// example nolimitconnect.xyz does not have a online id
	bool						setUrlHostType( EHostType hostType, bool forceHostType = false ); 
	static bool					setUrlHostType( std::string& url, EHostType hostType );

    void						clear( void );

protected:
    void                         parseHostType( void );

    std::string					m_Url;
	std::string					m_Protocol;
	std::string					m_Host;
	uint16_t					m_Port{ 0 };
	VxGUID						m_OnlineId;
	EHostType					m_HostType{ eHostTypeUnknown };
};


