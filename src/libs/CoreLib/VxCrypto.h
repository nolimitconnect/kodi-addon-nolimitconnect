#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxDefs.h"
#include "Blowfish.h"

#include <string>

//=== cheezy crypto with known weeknesses ===//

//TEMPORARY UNTIL REPLACE CHEEZY CRYPT WITH BETTER

//! macro returns true if length is a multiple of 16 and encryptable 
//! NOTE: see macro ROUND_TO_16BYTE_BOUNDRY for handy way to round length up to a multiple of 16 bytes
#define VxIsEncryptable( exp )	((((exp) & 0xf )||(0==(exp)))?0:1)

#define		CHEEZY_CHALLENGE_LEN	16
#define		CHEEZY_SYM_KEY_LEN		16		// length of symmetric key
#define		CHEEZY_KNOWN_TEXT_STR	"ads;&Okl*q3&O&T" // known string 16 chars long

class VxConnectId;
class VxGUID;

class VxKey
{
public:
	VxKey() = default;
	~VxKey();

	//! return true if key has been set
	bool						isKeySet( void ) { return m_bIsSet; }
	//! get key length in bytes
	int							getKeyLength( void ) { return 16; }
	//! validate is a valid length to encrypt or decrypt
	bool						isValidDataLen( int iDataLen ) { return ( iDataLen & 0x0f ) ? 0 : 1; }

	//! set key from data..
	int32_t						importKey( unsigned char * pu8Data, int iLen );
	//! set encryption key from another
	void						importKey( VxKey * poKey );
	//! return key into data buffer
	int32_t						exportKey( unsigned char * pu8RetKeyData, int iBufLen );
	//! export encryption key to another another
	void						exportKey( VxKey * poKey );
	void						exportToAsciiString( std::string& exportedKey );
	//! make encryption key from password
	int32_t						setKeyFromPassword( const char*	pPassword,			// password
													int			iPasswordLen,		// length of password
													const char*	pSalt = "NoLm" );	// salt
	//! make encryption key from user name and password 
	int32_t						setKeyFromPassword( const char*	pUserName,			// user name
													const char*	pPassword,			// password
													const char*	pSalt = "NoLm" );	// salt

	std::string					describeKey( void );

	//=== vars ===//
	uint32_t					m_au32Key[ CHEEZY_SYM_KEY_LEN / sizeof( uint32_t ) ];
	bool						m_bIsSet{ false };		// true if key has been set	
};

class VxCrypto
{
public:
	VxCrypto() = default;

	//! return true if key set and context generated
	bool						isKeyValid( void ) { return m_bIsKeyValid; }

	//! set key used for encryption and decryption
	void						setKey( VxKey * poKey ) { importKey( poKey ); }

	//! get key used for encryption and decryption
	VxKey *						getKey( void ) { return &m_Key; }

	//! set key used for encryption and decryption
	int32_t						importKey( VxKey * poKey );

	//! generate key from password and set encryption key in one function call
	//! NOTE: Max password len 255
	int32_t						setPassword( const char* pPassword, int iPasswordLen );

	//! Generate encryption key from password
	int32_t						generateKey( const char* pPassword, int iPasswordLen, VxKey * pgRetKey );

	//! encrypt some data
	//! NOTE: iDataLen must be a multiple of 16
	int32_t						encrypt( unsigned char * pu8Data, int iDataLen );

	//! decrypt some data
	//! NOTE: iDataLen must be a multiple of 16
	int32_t						decrypt( unsigned char * pu8Data, int iDataLen );

	//! encrypt known string
	int32_t						EncryptKnownString( unsigned char * pu8RetData, int iDataLen );

	//! verify data is known text string
	int32_t						VerifyKnownString( unsigned char * pu8Data, int iDataLen );

	//=== vars ===//
	BlowCtx						m_BlowCtx;				// context of blowfish
	VxKey						m_Key;					// encryption key
	bool						m_bIsKeyValid{ 0 };		// if true encryption key has been set
};

//============================================================================
// global functions
//============================================================================
//! fill with random data
//! NOTE: this is not truly random enough for strong encryption uses
void CheezyFillRandom( void * pvData, int iLen );

//============================================================================
//! encrypt data with VxCryptoo
int32_t VxSymEncrypt( VxKey *			poKey,			// Symmetric key must be 16 bytes long
	                char *			pData,			// buffer to encrypt
	                int				iDataLen,		// data length ( must be multiple of key length )
	                char *			pRetBuf = 0 );	// if null then encrypted data put in pData

//============================================================================
//! decrypt data with VxCryptoo
int32_t VxSymDecrypt( VxKey *			poKey,			// Symmetric key must be 16 bytes long
	                char *			pData,			// buffer to decrypt
	                int				iDataLen,		// data length ( must be multiple of key length )
	                char *			pRetBuf = 0 );	// if null then encrypted data put in pData

//============================================================================
bool GenerateConnectionKey(  VxKey *					poRetKey,		// set this key
							 VxConnectId *				poConnectId,	// network identity
							 uint16_t					cryptoPort,
							 const char*				networkName );

//============================================================================
bool GenerateConnectionKey(  VxKey *					poRetKey,		// set this key
                             std::string&               ipAddr,
                             uint16_t                   port,
                             VxGUID&                    onlineId,
                             uint16_t					cryptoPort,
                             std::string&               strNetName );
