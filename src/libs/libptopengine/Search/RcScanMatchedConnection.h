#pragma once
//============================================================================
// Copyright (C) 2016 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxDefs.h>

#include <memory>

class VxSktBase;
class VxNetIdent;

class RcScanMatchedConnection
{
public:
	RcScanMatchedConnection( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );
	virtual ~RcScanMatchedConnection() = default;

	RcScanMatchedConnection( const RcScanMatchedConnection& rhs );
	RcScanMatchedConnection& operator=( const RcScanMatchedConnection& rhs );

	void						setActionStartTimeMs( int64_t actionStartTime )		{ m_ActionStartTimeMs = actionStartTime; }
	uint64_t					getActionStartTimeMs( void )						{ return m_ActionStartTimeMs; }

	void						setActionHadError( bool hadError )				{ m_ActionHadError = hadError; if( hadError) m_ActionComplete = true; }
	bool						getActionHadError( void )						{ return m_ActionHadError; }

	void						setIsActionCompleted( bool completed )			{ m_ActionComplete = completed; }
	bool						getIsActionCompleted( void )					{ return m_ActionComplete; }

	void						deleteResources( void );

	//=== vars ===//
	VxNetIdent*					m_Ident{ nullptr };
	std::shared_ptr<VxSktBase>	m_Skt;
	uint64_t					m_ActionStartTimeMs{ 0 };
	uint8_t *					m_u8JpgData{ nullptr };
	uint32_t					m_u32JpgDataLen{ 0 };
	bool						m_ActionHadError{ false };
	bool						m_ActionComplete{ false };
};