#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxDefs.h"
#include <string>
#include <memory.h>

#ifdef __cplusplus

constexpr int FILE_HASH_LEN_BYTES = 20;

#pragma pack(push)
#pragma pack(1)
class VxThread;
//! 20 bytes in size
class VxSha1Hash
{
public:
	VxSha1Hash();
	VxSha1Hash( uint8_t * data ) { setHashData( data ); }
	VxSha1Hash( const VxSha1Hash &rhs );
	VxSha1Hash&					operator =( const VxSha1Hash &a );
	bool						operator == ( const VxSha1Hash &a ) const;
	bool						operator != ( const VxSha1Hash &a ) const;

	bool						generateHashFromFile( const char* fileName, VxThread* workThread = 0 );
	bool						isHashValid( void )	const;
	bool						isEqualTo(  const uint8_t * hashData ) const;

	void						setHashData( const uint8_t * hashData );
	uint8_t *					getHashData( void )						{ return m_HashId; }

	std::string					toString( void ) const;
	void						clear( void ) { memset( m_HashId, 0, sizeof( m_HashId ) ); }

protected:
	char						highNibbleChar( uint8_t val ) const		{ return nibbleToHex( val >> 4 ); }
    char						lowNibbleChar( uint8_t val ) const		{ return nibbleToHex( val & 0x0F ); }
	char						nibbleToHex( uint8_t val ) const		{ return val > 9 ? (val - 10) + 'A' : val + 0x30; }

	uint8_t						m_HashId[FILE_HASH_LEN_BYTES];
};
#pragma pack(pop)

#endif // __cplusplus
