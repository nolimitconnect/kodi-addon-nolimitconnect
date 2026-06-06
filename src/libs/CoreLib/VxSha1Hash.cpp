
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSha1Hash.h"

#include "SHA1.h"
#include "VxDebug.h"

//============================================================================
VxSha1Hash::VxSha1Hash()
{
	memset( m_HashId, 0, sizeof( m_HashId ) );
}

//============================================================================
VxSha1Hash::VxSha1Hash( const VxSha1Hash &rhs )
{
	*this = rhs;
}

//============================================================================
VxSha1Hash& VxSha1Hash::operator =( const VxSha1Hash &rhs )
{
	if( this != &rhs )   
	{
		memcpy( m_HashId, rhs.m_HashId, sizeof( m_HashId ) );
	}

	return *this;
}

//============================================================================
bool VxSha1Hash::operator == ( const VxSha1Hash &a ) const
{
	return ( 0 == memcmp( m_HashId, a.m_HashId, sizeof( m_HashId ) ) ) ? true : false;
}

//============================================================================
bool VxSha1Hash::operator != ( const VxSha1Hash &a ) const
{
	return ! (*this == a);
}

//============================================================================
bool VxSha1Hash::isHashValid()	const
{
	uint32_t * longPtr = (uint32_t *)m_HashId;
	for( unsigned int i = 0; i < 4; i++ )
	{
		if( 0 != longPtr[0] )
		{
			return true;
		}
	}

	return false;
}

//============================================================================
bool VxSha1Hash::isEqualTo(  const uint8_t * hashData ) const
{
	bool isEqual = false;
	if( hashData )
	{
		isEqual = (0 == memcmp( hashData, m_HashId, sizeof( m_HashId ) ) );
	}

	return isEqual;
}


//============================================================================
void VxSha1Hash::setHashData( const uint8_t * data )			
{ 
	if( data )
	{
		memcpy( m_HashId, data, sizeof( m_HashId ) ); 
	}
	else
	{
		memset( m_HashId, 0, sizeof( m_HashId ) ); 
	}
}

//============================================================================
bool VxSha1Hash::generateHashFromFile( const char* fileName, VxThread* workThread )
{
	CSHA1 shaInstance;
	memset( m_HashId, 0, sizeof( m_HashId ) );
	bool result = shaInstance.HashFile( fileName, workThread );
	if( result )
	{
		shaInstance.Final();
		shaInstance.GetHash( m_HashId );
	}
	else
	{
		LogMsg( LOG_ERROR, "%s failed to generate hash for file %s", __func__, fileName );
	}

	return result;
}

//============================================================================
std::string VxSha1Hash::toString( void ) const
{
	char buf[ 41 ];
	int idx = 0;
	for( int i = 0; i < 20; i++ )
	{
		uint8_t byte = m_HashId[ i ];
		buf[ idx ] = highNibbleChar( byte );
		idx++;
		buf[ idx ] = lowNibbleChar( byte );
        idx++;
	}

	buf[ idx ] = 0;
	return buf;
}
