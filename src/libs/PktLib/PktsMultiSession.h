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

#include "VxPktHdr.h"

#pragma pack(push)
#pragma pack(1)

class PktMultiSessionReq : public VxPktHdr
{
public:
	PktMultiSessionReq();

	void						        setMSessionAction( uint32_t action )		{ m_u32Action = htonl( action ); }
	uint32_t							getMSessionAction( void )			        { return ntohl( m_u32Action ); }
	void						        setMSessionParam( uint32_t param )		    { m_u32ActionParam = htonl( param ); }
	uint32_t							getMSessionParam( void )			        { return ntohl( m_u32ActionParam ); }

private:
	//=== vars ===//
	uint32_t							m_u32Action;
	uint32_t							m_u32ActionParam;
	uint32_t							m_u32Res3;
	uint32_t							m_u32Res4;
	uint32_t							m_u32Res5;
	uint32_t							m_u32Res6;
};

class PktMultiSessionReply : public VxPktHdr
{
public:
	PktMultiSessionReply();

	void						        setMSessionAction( uint32_t action )		{ m_u32Action = htonl( action ); }
	uint32_t							getMSessionAction( void )			        { return ntohl( m_u32Action ); }
	void						        setMSessionParam( uint32_t param )		    { m_u32ActionParam = htonl( param ); }
	uint32_t							getMSessionParam( void )			        { return ntohl( m_u32ActionParam ); }

private:
	//=== vars ===//
	uint32_t							m_u32Action;
	uint32_t							m_u32ActionParam;
	uint32_t							m_u32Res3;
	uint32_t							m_u32Res4;
	uint32_t							m_u32Res5;
	uint32_t							m_u32Res6;
};

#pragma pack(pop)

