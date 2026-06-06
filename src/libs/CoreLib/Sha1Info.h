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

#include <CoreLib/VxSha1Hash.h>
#include <CoreLib/VxGUID.h>

#include <string>

class Sha1Info
{
public:
	Sha1Info() = default;
	Sha1Info( VxGUID& assetId, std::string& fileName, std::string& fileNameAndPath );
    Sha1Info( const Sha1Info& rhs );
	~Sha1Info() = default;

    Sha1Info&                   operator = ( const Sha1Info& rhs );
    bool                        operator == ( const Sha1Info& rhs ) const;

	bool						isValid( bool checkHashValid = true );

	void						setSha1Hash( VxSha1Hash& sha1Hash )			{m_Sha1Hash = sha1Hash; }
	VxSha1Hash&					getSha1Hash( void )							{ return m_Sha1Hash; }

	void						setAssetId( VxGUID& assetId )				{ m_AssetId = assetId; }
	VxGUID&						getAssetId( void )							{ return m_AssetId; }

	void						setFileLen( int64_t fileLen )				{ m_FileLen = fileLen; }
	int64_t						getFileLen( void )							{ return m_FileLen; }

	void						setFileName( std::string& fileName )		{ m_FileName = fileName; }
	std::string&				getFileName( void )							{ return m_FileName; }

	void						setFileNameAndPath( std::string& fileNameAndPath )	{ m_FileNameAndPath = fileNameAndPath; }
	std::string&				getFileNameAndPath( void )							{ return m_FileNameAndPath; }

protected:
	VxSha1Hash					m_Sha1Hash;
	VxGUID						m_AssetId;
	uint64_t					m_FileLen{ 0 };
    std::string					m_FileName;
	std::string					m_FileNameAndPath;
};


