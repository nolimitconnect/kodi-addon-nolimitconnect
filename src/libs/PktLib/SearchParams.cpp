//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SearchParams.h"
#include <CoreLib/PktBlobEntry.h>

#include <CoreLib/VxTime.h>

//============================================================================
SearchParams::SearchParams( const SearchParams& rhs )
    : MatchParams(rhs)
    , m_HostType( rhs.m_HostType )
    , m_SearchType( rhs.m_SearchType )
    , m_SearchStartTimeGmtMs( rhs.m_SearchStartTimeGmtMs )
    , m_SearchSessionId( rhs.m_SearchSessionId )
    , m_SearchIdentGuid(rhs.m_SearchIdentGuid )
    , m_SearchUrl( rhs.m_SearchUrl )
    , m_SearchText( rhs.m_SearchText )
    , m_SearchListAll( rhs.m_SearchListAll )
{
}

//============================================================================
SearchParams& SearchParams::operator =( const SearchParams& rhs )
{
	if( this != &rhs )
	{
        *((MatchParams *)this) = (const MatchParams&)rhs;

        m_HostType              = rhs.m_HostType;
        m_SearchType            = rhs.m_SearchType;
        m_SearchStartTimeGmtMs  = rhs.m_SearchStartTimeGmtMs;
        m_SearchSessionId       = rhs.m_SearchSessionId;
        m_SearchIdentGuid       = rhs.m_SearchIdentGuid;
        m_SearchUrl		        = rhs.m_SearchUrl;
        m_SearchText		    = rhs.m_SearchText;
        m_SearchListAll         = rhs.m_SearchListAll;
	}

	return *this;
}

//============================================================================
bool SearchParams::addToBlob( PktBlobEntry& blob )
{
    bool result = MatchParams::addToBlob( blob );
    result &= blob.setValue( m_HostType );
    result &= blob.setValue( m_SearchType );
    result &= blob.setValue( m_SearchStartTimeGmtMs );
    result &= blob.setValue( m_SearchSessionId );
    result &= blob.setValue( m_SearchIdentGuid );
    result &= blob.setValue( m_SearchUrl );
    result &= blob.setValue( m_SearchText );
    result &= blob.setValue( m_SearchListAll );
    return result;
}

//============================================================================
bool SearchParams::extractFromBlob( PktBlobEntry& blob )
{
    bool result = MatchParams::extractFromBlob( blob );
    result &= blob.getValue( m_HostType );
    result &= blob.getValue( m_SearchType );
    result &= blob.getValue( m_SearchStartTimeGmtMs );
    result &= blob.getValue( m_SearchSessionId );
    result &= blob.getValue( m_SearchIdentGuid );
    result &= blob.getValue( m_SearchUrl );
    result &= blob.getValue( m_SearchText );
    result &= blob.getValue( m_SearchListAll );
    return result;
}

//============================================================================
void SearchParams::createNewSessionId( void )
{
    VxGUID::generateNewVxGUID( m_SearchSessionId );
}

//============================================================================
void SearchParams::updateSearchStartTime( void )
{
    m_SearchStartTimeGmtMs = GetGmtTimeMs();
}