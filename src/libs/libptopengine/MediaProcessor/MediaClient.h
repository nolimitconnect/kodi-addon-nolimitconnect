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

#include <GuiInterface/IFromGui.h>
#include <CoreLib/MediaCallbackInterface.h>

class MediaClient
{
public:
	MediaClient(	VxGUID&						onlineId,
					EMediaModule				mediaModule,
					EMediaInputType				mediaType, 
					MediaCallbackInterface*		callback,
					VxGUID						sessionId )
	: m_OnlineId( onlineId )
	, m_MediaModule( mediaModule )
	, m_MediaInputType( mediaType )
	, m_Callback( callback )
	, m_SessionId( sessionId )
	{
	}

	MediaClient( const MediaClient &rhs )
	{
		if( this != &rhs )
		{
			*this = rhs;
		}
	}

	MediaClient&				operator =( const MediaClient &rhs )
	{
		if( this != &rhs )
		{
			m_OnlineId			= rhs.m_OnlineId;
			m_MediaModule		= rhs.m_MediaModule;
			m_MediaInputType	= rhs.m_MediaInputType;
			m_Callback			= rhs.m_Callback;
			m_SessionId			= rhs.m_SessionId;
		}

		return *this;
	}

	VxGUID						m_OnlineId;
	EMediaModule				m_MediaModule;
	EMediaInputType				m_MediaInputType;
	MediaCallbackInterface *	m_Callback; 
	VxGUID						m_SessionId;
};
