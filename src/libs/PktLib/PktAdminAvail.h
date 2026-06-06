#pragma once

//============================================================================
// Copyright (C) 2025 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxPktHdr.h"

#include <CoreLib/GroupieId.h>
#include <GuiInterface/IDefs.h>

#pragma pack(push)
#pragma pack(1)
class PktAdminAvail : public VxPktHdr
{
public:
	PktAdminAvail();

	void						setAdminAvailable( bool adminAvail )			{ m_AdminAvail = ( uint8_t	)adminAvail; };
	uint8_t						getAdminAvailable( void )						{ return m_AdminAvail; }

	void						setAdminGroupieId( GroupieId adminId )			{ m_AdminGroupieId = adminId; };
	GroupieId					getAdminGroupieId( void )						{ return m_AdminGroupieId; }

private:
	//=== vars ===//
	uint8_t						m_AdminAvail{ 0 };
	uint16_t					m_Res1{ 0 };
	uint32_t					m_Res2{ 0 };
	GroupieId					m_AdminGroupieId;
};

#pragma pack(pop)
