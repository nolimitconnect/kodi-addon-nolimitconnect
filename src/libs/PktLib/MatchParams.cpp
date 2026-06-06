//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "MatchParams.h"
#include <CoreLib/PktBlobEntry.h>

//============================================================================
MatchParams::MatchParams( const MatchParams& rhs )
    : m_AgeType(rhs.m_AgeType)
    , m_GenderType(rhs.m_GenderType)
    , m_LanguageType(rhs.m_LanguageType)
    , m_ContentRating(rhs.m_ContentRating)
{
}

//============================================================================
MatchParams& MatchParams::operator =( const MatchParams& rhs )
{
	if( this != &rhs )
	{
        m_AgeType               = rhs.m_AgeType;
        m_GenderType            = rhs.m_GenderType;
        m_LanguageType          = rhs.m_LanguageType;
        m_ContentRating         = rhs.m_ContentRating;
	}

	return *this;
}

//============================================================================
bool MatchParams::addToBlob( PktBlobEntry& blob )
{
    bool result = blob.setValue( m_AgeType );
    result &= blob.setValue( m_GenderType );
    result &= blob.setValue( m_LanguageType );
    result &= blob.setValue( m_ContentRating );
    return result;
}

//============================================================================
bool MatchParams::extractFromBlob( PktBlobEntry& blob )
{
    bool result = blob.getValue( m_AgeType );
    result &= blob.getValue( m_GenderType );
    result &= blob.getValue( m_LanguageType );
    result &= blob.getValue( m_ContentRating );
    return result;
}