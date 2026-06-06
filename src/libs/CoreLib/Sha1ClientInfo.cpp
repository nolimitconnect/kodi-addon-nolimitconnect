//============================================================================
// Copyright (C) 2022 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "Sha1ClientInfo.h"

#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxGUID.h>

#include <CoreLib/VxDebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//============================================================================
Sha1ClientInfo::Sha1ClientInfo( VxGUID& fileId, std::string& fileName, std::string& fileNameAndPath,  Sha1GeneratorCallback* client )
    : m_Sha1Info( fileId, fileName, fileNameAndPath )
    , m_Client( client )
{
}

//============================================================================
Sha1ClientInfo::Sha1ClientInfo( const Sha1Info& sha1Info, Sha1GeneratorCallback* client )
    : m_Sha1Info( sha1Info )
    , m_Client( client )
{
}

//============================================================================
Sha1ClientInfo::Sha1ClientInfo( const Sha1ClientInfo& rhs )
    : m_Sha1Info( rhs.m_Sha1Info )
    , m_Client( rhs.m_Client )
{
}

//============================================================================
Sha1ClientInfo& Sha1ClientInfo::operator = ( const Sha1ClientInfo& rhs )
{
    if( this != &rhs )
    {
        m_Sha1Info = rhs.m_Sha1Info;
        m_Client = rhs.m_Client;
    }

    return *this;
}

//============================================================================
bool Sha1ClientInfo::operator == ( const Sha1ClientInfo& rhs ) const
{
    return m_Sha1Info == rhs.m_Sha1Info && m_Client == rhs.m_Client;
}

//============================================================================
bool Sha1ClientInfo::isValid( bool checkHashValid )
{
    return m_Client != nullptr && m_Sha1Info.isValid( checkHashValid );
}
