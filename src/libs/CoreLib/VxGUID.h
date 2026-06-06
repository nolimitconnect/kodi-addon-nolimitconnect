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

#include "VxDefs.h"
#include <string>

#pragma pack(push)
#pragma pack(1)

class PktBlobEntry;

//! implements unique identifier
//! 16 bytes in size

class VxGUID
{
public:
	VxGUID() = default;
	VxGUID( VxGUID& rhs );
	VxGUID( const VxGUID& rhs );
	VxGUID( uint64_t u64HiPart, uint64_t u64LoPart );
    VxGUID( const char * hexOrOnlineIdStr );

	VxGUID&						operator = ( const VxGUID& rhs );
	bool						operator == ( const VxGUID& rhs ) const;
	bool						operator != ( const VxGUID& rhs ) const;
	bool						operator < ( const VxGUID& rhs ) const;
	bool						operator <= ( const VxGUID& rhs ) const;
	bool						operator > ( const VxGUID& rhs ) const;
	bool						operator >= ( const VxGUID& rhs ) const;

	static void					generateNewVxGUID( VxGUID& retNewGUID );
	static VxGUID&				nullVxGUID( void );
    static VxGUID&				anybodyVxGUID( void );

	void						initializeWithNewVxGUID( void );
	void						clearVxGUID( void );

	bool						isValid()	const;
	void						assureIsValidGUID( void ) { if( !isValid() ) initializeWithNewVxGUID(); }
	std::string					toGUIDStandardFormatedString( void );

	bool						toHexString( std::string& strRetId ) const;
    void                        toHexString( char * retBuf ) const; // buffer must be at least 33 characters in length
	std::string					toHexString( void ) const;
	std::string					toAsci( void ) const;

    //! set VxGUID by converting hex string into VxGUID
    bool						fromVxGUIDHexString( const char* hexString );
	bool						fromRawData( uint8_t* rawData );
    static bool					isVxGUIDHexStringValid( const char* hexString );

    bool						toOnlineIdString( std::string& strRetId ) const;  
    void                        toOnlineIdString( char * retBuf ) const;  // buffer must be at least 35 characters in length
    std::string					toOnlineIdString( void ) const;

    //! set VxGUID by converting online id string into VxGUID
    bool						fromOnlineIdString( const char* onlineIdString );
    static bool					isOnlineIdStringValid( const char* onlineIdString );

	//! get low part of online id
	uint64_t					getVxGUIDLoPart() const;
	//! get high part of online id
	uint64_t					getVxGUIDHiPart() const;
	//! return users online id
	VxGUID&						getVxGUID( void );
	bool 						getVxGUID( std::string& strRetId );
	void						setVxGUID( const char* pId );
	void						setVxGUID( VxGUID& oId );
	void						setVxGUID( uint64_t& u64HiPart, uint64_t& u64LoPart );
	void						getVxGUID( uint64_t& u64HiPart, uint64_t& u64LoPart );
	// returns 0 if equal else -1 if less or 1 if greater
	int							compareTo( VxGUID& guid );
	// returns true if guids are same value
	bool						isEqualTo( const VxGUID& guid );

	std::string					describeVxGUID( bool asOnlineId = true );

    // set bytes to network order
    void                        setToNetOrder( void );
    // set bytes to host cpu endianess
    void                        setToHostOrder( void );

    // buffer must be at least 17 characters in length
    static void                 uint64ToHexAscii( char * retBuf, const uint64_t& val );
    static char                 nibbleToHex( uint8_t val );

	// fill wie random data which is faster but does NOT make it unique
	void						fillRandom( void );

	void						clear( void ) { m_u64HiPart = 0; m_u64LoPart = 0; }

protected:
    uint64_t					m_u64HiPart{ 0 };
	uint64_t					m_u64LoPart{ 0 };
};

#pragma pack(pop)

//! comparison of VxGUID ids for std::map
struct cmp_vxguid 
{
	bool operator()(const VxGUID& a, const VxGUID& b) const
	{
		//! return true if a < b
		return a < b;
	}
};

