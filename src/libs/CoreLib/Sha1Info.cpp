//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "Sha1Info.h"
#include "VxFileUtil.h"

//============================================================================
Sha1Info::Sha1Info( VxGUID& assetId, std::string& fileName, std::string& fileNameAndPath )
    : m_Sha1Hash()
    , m_AssetId( assetId )
    , m_FileName( fileName )
    , m_FileNameAndPath( fileNameAndPath )
{
   m_FileLen = VxFileUtil::fileExists( fileNameAndPath.c_str() );
}

//============================================================================
Sha1Info::Sha1Info( const Sha1Info& rhs )
    : m_Sha1Hash( rhs.m_Sha1Hash )
    , m_AssetId( rhs.m_AssetId )
    , m_FileLen( rhs.m_FileLen )
    , m_FileName( rhs.m_FileName )
    , m_FileNameAndPath( rhs.m_FileNameAndPath )
{
}

//============================================================================
Sha1Info& Sha1Info::operator = ( const Sha1Info& rhs )
{
    if( this != &rhs )
    {
        m_Sha1Hash = rhs.m_Sha1Hash;
        m_AssetId = rhs.m_AssetId;
        m_FileLen = rhs.m_FileLen;
        m_FileName = rhs.m_FileName;
        m_FileNameAndPath = rhs.m_FileNameAndPath;
    }

    return *this;
}

//============================================================================
bool Sha1Info::operator == ( const Sha1Info& rhs ) const
{
    return m_Sha1Hash == rhs.m_Sha1Hash && m_AssetId == rhs.m_AssetId 
        && m_FileName == rhs.m_FileName && m_FileNameAndPath == rhs.m_FileNameAndPath && m_FileLen == rhs.m_FileLen;
}

//============================================================================
bool Sha1Info::isValid( bool checkHashValid )
{
    bool isValid = m_AssetId.isValid() && m_FileLen && !m_FileName.empty() && !m_FileNameAndPath.empty();
    if( checkHashValid )
    {
        isValid &= m_Sha1Hash.isHashValid();
    }

    return isValid;
}
