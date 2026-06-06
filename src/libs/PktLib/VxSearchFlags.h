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

#include <inttypes.h>

#pragma pack(push)
#pragma pack(1)

class PktBlobEntry;
//! 2 bytes in size
class VxSearchFlags
{
public:
	VxSearchFlags() = default;
    VxSearchFlags( const VxSearchFlags& rhs );
    VxSearchFlags&              operator =( const VxSearchFlags& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	void						setSearchFlags( uint8_t flags )			{ m_u8SearchFlags = flags; }
	uint8_t						getSearchFlags( void )				    { return m_u8SearchFlags; }
	void						setSharedFileTypes( uint8_t flags )		{ m_u8FileTypeFlags = flags; }
	uint8_t						getSharedFileTypes( void )			    { return m_u8FileTypeFlags; }

	bool 						hasSearchFlags( void );
	bool						hasSharedFiles( void );

	void						setHasPhotoFiles( bool bHasFiles );
	bool						hasPhotoFiles( void );

	void						setHasAudioFiles( bool bHasFiles );
	bool						hasAudioFiles( void );

	void						setHasVideoFiles( bool bHasFiles );
	bool						hasVideoFiles( void );

	void						setHasDocFiles( bool bHasFiles );
	bool						hasDocFiles( void );

	void						setHasArcOrCdImageFiles( bool bHasFiles );
	bool						hasArcOrCdFiles( void );

	void						setHasOtherFiles( bool bHasFiles );
	bool						hasOtherFiles( void );

	void						setHasAboutMeContent( bool hasContent );
	bool						hasAboutMeContent( void );

	void						setHasStoryboardContent( bool hasContent );
	bool						hasStoryboardContent( void );

	void						setHasSharedWebCam( bool bHasWebCam );
	bool						hasSharedWebCam( void );

protected:
	//=== vars ===//
    uint8_t						m_u8SearchFlags{ 0 };
	uint8_t						m_u8FileTypeFlags{ 0 };
};

#pragma pack(pop)

