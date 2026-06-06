#pragma once
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

#include <CoreLib/VxGUID.h>

class SearchParams : public MatchParams
{
public:
	SearchParams() = default;
    SearchParams( const SearchParams& rhs );
	virtual ~SearchParams() = default;

	SearchParams&				operator =( const SearchParams& rhs );

    void						setHostType( enum EHostType hostType )                      { m_HostType = hostType; }
    EHostType					getHostType( void )								            { return m_HostType; }

    void						setSearchType( enum ESearchType searchType )				{ m_SearchType = searchType; }
    ESearchType					getSearchType( void )								        { return m_SearchType; }

    // search session (unique per search instance)
    void						setSearchSessionId( VxGUID& guid )						    { m_SearchSessionId = guid; }
    VxGUID&					    getSearchSessionId( void )								    { return m_SearchSessionId; }
    void						createNewSessionId( void );
    void						updateSearchStartTime( void );

    // search for identity (for person or host)
    void						setSearchIdentGuid( VxGUID& guid )							{ m_SearchIdentGuid = guid; }
    VxGUID&					    getSearchIndentGuid( void )									{ return m_SearchIdentGuid; }

    void						setSearchUrl( std::string& url )							{ m_SearchUrl = url; }
    std::string&				getSearchUrl( void )									    { return m_SearchUrl; }

	void						setSearchText( std::string& text )							{ m_SearchText = text; }
    std::string&				getSearchText( void )									    { return m_SearchText; }

    virtual void                setSearchListAll( bool listAll )                            { m_SearchListAll = listAll; }
    virtual bool                getSearchListAll( void )                                    { return m_SearchListAll; }

    virtual bool                addToBlob( PktBlobEntry& blob ) override;
    virtual bool                extractFromBlob( PktBlobEntry& blob ) override;

protected:
	//=== vars ===//
    EHostType                   m_HostType{ eHostTypeUnknown };
    ESearchType                 m_SearchType{ eSearchNone };
    uint64_t                    m_SearchStartTimeGmtMs{ 0 };
    VxGUID					    m_SearchSessionId;
    VxGUID					    m_SearchIdentGuid;
    std::string					m_SearchUrl;
	std::string					m_SearchText;
    bool                        m_SearchListAll{ false };
};


