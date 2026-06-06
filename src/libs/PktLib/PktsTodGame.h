#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PktTypes.h"
#include "VxCommon.h"

#include <GuiInterface/IDefs.h>

#define PIC_TYPE_JPEG				0x01

enum ETodGameVarId
{
	eTodGameVarIdDareChoiceCnt,
	eTodGameVarIdDareAcceptedCnt,
	eTodGameVarIdDareRejectedCnt,
	eTodGameVarIdTruthChoiceCnt,
	eTodGameVarIdTruthAcceptedCnt,
	eTodGameVarIdTruthRejectedCnt,

	eMaxTodGameStatId,
	eMaxTodGameVarId
};

#pragma pack(push)
#pragma pack(1)

// NOTE: no longer used
class PktTodGameStats : public VxPktHdr
{
public:
	PktTodGameStats();

    void                            setVar( enum ETodGameVarId varId, uint32_t varValue );
    uint32_t						getVar( enum ETodGameVarId varId );

private:
	//=== vars ===//
	uint32_t						m_DareChallengeCnt{ 0 };
	uint32_t						m_DareAcceptedCnt{ 0 };
	uint32_t						m_DareRejectedCnt{ 0 };
	uint32_t						m_TruthChallengeCnt{ 0 };
	uint32_t						m_TruthAcceptedCnt{ 0 };
	uint32_t						m_TruthRejectedCnt{ 0 };
	uint32_t						m_u32Res1{ 0 };
	uint32_t						m_u32Res2{ 0 };
	uint32_t						m_u32Res3{ 0 };
	uint32_t						m_u32Res4{ 0 };
};

class PktTodGameAction : public VxPktHdr
{
public:
	PktTodGameAction();

    void						setAction( enum ETodGameAction eAction, int32_t s32Val );
	ETodGameAction				getActionVarId( void );
    int32_t						getActionVarValue( void );

private:
	//=== vars ===//
    int32_t						m_s32ActionId{ 0 };
    int32_t						m_s32ActionVal{ 0 };
    uint32_t					m_u32Res1{ 0 };
    uint32_t					m_u32Res2{ 0 };
    uint32_t					m_u32Res3{ 0 };
    uint32_t					m_u32Res4{ 0 };
};

// NOTE: no longer used
class PktTodGameValue : public VxPktHdr
{
public:
	PktTodGameValue();

    void						setValue( enum ETodGameVarId eValueId, int32_t s32Val );

	ETodGameVarId				getValueVarId( void );

    int32_t						getValueVar( void );

private:
	//=== vars ===//
    int32_t						m_s32GameValueId{ 0 };
    int32_t						m_s32GameValueVar{ 0 };
    uint32_t					m_u32Res1{ 0 };
    uint32_t					m_u32Res2{ 0 };
    uint32_t					m_u32Res3{ 0 };
    uint32_t					m_u32Res4{ 0 };
};

#pragma pack(pop)
