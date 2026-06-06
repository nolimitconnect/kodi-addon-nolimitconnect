#pragma once
//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/Sha1GeneratorCallback.h>
#include <CoreLib/Sha1Info.h>

class Sha1ClientInfo
{
public:
	Sha1ClientInfo() = default;
	Sha1ClientInfo( VxGUID& fileId, std::string& fileName, std::string& fileNameAndPath, Sha1GeneratorCallback* client );
	Sha1ClientInfo( const Sha1Info& sha1Info, Sha1GeneratorCallback* client );
    Sha1ClientInfo( const Sha1ClientInfo& rhs );
	~Sha1ClientInfo() = default;

    Sha1ClientInfo&             operator = ( const Sha1ClientInfo& rhs );
    bool                        operator == ( const Sha1ClientInfo& rhs ) const;

	bool						isValid( bool checkHashValid = true );

	void						setSha1Info( Sha1Info& sha1Info )			{ m_Sha1Info = sha1Info; }
	Sha1Info&					getSha1Info( void )							{ return m_Sha1Info; }

	void						setClient( Sha1GeneratorCallback* client )  { m_Client = client; }
	Sha1GeneratorCallback*		getClient( void )							{ return m_Client; }

	std::string&				getFileName( void )							{ return m_Sha1Info.getFileName(); }
	std::string&				getFileNameAndPath( void )					{ return m_Sha1Info.getFileNameAndPath(); }
	VxGUID&						getAssetId( void )							{ return m_Sha1Info.getAssetId(); }

protected:
	Sha1Info					m_Sha1Info;
	Sha1GeneratorCallback*		m_Client{ nullptr };
};


