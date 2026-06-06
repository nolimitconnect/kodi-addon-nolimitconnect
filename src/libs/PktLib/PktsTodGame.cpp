//============================================================================
// Copyright (C) 2010 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktsTodGame.h"

#include <CoreLib/VxDebug.h>

//============================================================================
PktTodGameStats::PktTodGameStats()
{
	setPktLength( sizeof( PktTodGameStats ) ); 
	setPktType( PKT_TYPE_TOD_GAME_STATS ); 
}

//============================================================================
uint32_t PktTodGameStats::getVar( ETodGameVarId eVarId )
{
	switch(eVarId)
	{
	case eTodGameVarIdDareChoiceCnt:
		return ntohl( m_DareChallengeCnt );
	case eTodGameVarIdDareAcceptedCnt:
		return ntohl( m_DareAcceptedCnt );
	case eTodGameVarIdDareRejectedCnt:
		return ntohl( m_DareRejectedCnt );
	case eTodGameVarIdTruthChoiceCnt:
		return ntohl( m_TruthChallengeCnt );
	case eTodGameVarIdTruthAcceptedCnt:
		return ntohl( m_TruthAcceptedCnt );
	case eTodGameVarIdTruthRejectedCnt:
		return ntohl( m_TruthRejectedCnt );
	default:
		return 0;
	}
}

//============================================================================
void PktTodGameStats::setVar( ETodGameVarId eVarId, uint32_t s32Value )
{
	switch(eVarId)
	{
	case eTodGameVarIdDareChoiceCnt:
		m_DareChallengeCnt = htonl( s32Value );
		break;
	case eTodGameVarIdDareAcceptedCnt:
		m_DareAcceptedCnt = htonl( s32Value );
		break;
	case eTodGameVarIdDareRejectedCnt:
		m_DareRejectedCnt = htonl( s32Value );
		break;
	case eTodGameVarIdTruthChoiceCnt:
		m_TruthChallengeCnt = htonl( s32Value );
		break;
	case eTodGameVarIdTruthAcceptedCnt:
		m_TruthAcceptedCnt = htonl( s32Value );
		break;
	case eTodGameVarIdTruthRejectedCnt:
		m_TruthRejectedCnt = htonl( s32Value );
		break;
	default:
		break;
	}
}

//============================================================================
PktTodGameAction::PktTodGameAction()
{
	setPktLength( sizeof( PktTodGameAction ) ); 
	setPktType( PKT_TYPE_TOD_GAME_ACTION ); 
}

//============================================================================
void PktTodGameAction:: setAction( ETodGameAction eAction, int32_t s32Val )
{
	m_s32ActionId	= htonl( eAction );
	m_s32ActionVal	= htonl( s32Val );
}

//============================================================================
ETodGameAction PktTodGameAction::getActionVarId( void )
{
	return (ETodGameAction) ntohl( m_s32ActionId );
}

//============================================================================
int32_t PktTodGameAction::getActionVarValue( void )
{
	return ntohl( m_s32ActionVal );
}

//============================================================================
PktTodGameValue::PktTodGameValue()
{
	setPktLength( sizeof( PktTodGameValue ) ); 
	setPktType( PKT_TYPE_TOD_GAME_VALUE ); 
}

//============================================================================
void PktTodGameValue::setValue( ETodGameVarId eValueId, int32_t s32Val )
{
	m_s32GameValueId	= htonl( eValueId );
	m_s32GameValueVar	= htonl( s32Val );
}

//============================================================================
ETodGameVarId PktTodGameValue::getValueVarId( void )
{
	return (ETodGameVarId) ntohl( m_s32GameValueId );
}

//============================================================================
int32_t PktTodGameValue::getValueVar( void )
{
	return ntohl( m_s32GameValueVar );
}

