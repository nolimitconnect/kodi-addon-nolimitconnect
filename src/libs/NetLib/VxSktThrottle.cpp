//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktThrottle.h"

#include <CoreLib/VxDebug.h>

//============================================================================

VxSktThrottle::VxSktThrottle()
	: m_bEnableThrottle( false )
	, m_f32TxSpeedLimit( 0.0f ) 
	, m_f32RxSpeedLimit( 0.0f ) 
{
	ResetStatistics();
}

//============================================================================

VxSktThrottle::~VxSktThrottle()
{

}

//============================================================================
//! reset all statistics
void VxSktThrottle::ResetStatistics( void )
{
	ResetRxStatistics();
	ResetTxStatistics();
}

//============================================================================
//! reset just tx statistics
void VxSktThrottle::ResetTxStatistics( void )
{
	m_iTxSamplesCnt = 0;
	m_iTxSamplesBytes = 0;
	m_s64BytesTxed = 0;
	m_s64TxPktCnt = 0;
	m_f32TxCurrentSpeed = 0.0f; 
	m_TxTimer.startTimer();
}

//============================================================================
//! reset just send statistics
void VxSktThrottle::ResetRxStatistics( void )
{
	m_iRxSamplesCnt = 0;
	m_iTxSamplesBytes = 0;
	m_s64BytesRxed = 0;
	m_s64RxPktCnt = 0;
	m_f32TxCurrentSpeed = 0.0f; 
	m_RxTimer.startTimer();
}

//============================================================================
//! packet was sent
void VxSktThrottle::TxedPkt( int iPktLen )
{
	vx_assert( iPktLen >= 0 );
	m_s64BytesTxed += iPktLen;
	m_s64TxPktCnt++;
	//m_iTxSamplesBytes += iPktLen;
	//m_iTxSamplesCnt++;
	//if( VxSKT_INFO_SAMPLE_CNT == m_iTxSamplesCnt )
	//{
	//	// compute speed
	//	UpdateTxSpeed();
	//	// reset sample info
	//	m_iTxSamplesBytes = 0;
	//	m_iTxSamplesCnt = 0;
	//	m_TxTimer.startTimer();
	//}
}

//============================================================================
//! packet was recieved
void VxSktThrottle::RxedPkt( int iPktLen )
{
	m_s64BytesRxed += iPktLen;
	m_s64RxPktCnt++;
}
