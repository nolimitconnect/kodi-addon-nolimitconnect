#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxDefs.h"

extern unsigned char	aucCommonToChar[];
extern unsigned char	aucCharToCommon[];
extern uint16_t				aucCommonToWChar[];
extern uint16_t				aucWCharToCommon[];

void					VxMapStr( char * pStr );
void					VxUnMapStr( char * pStr );
void					VxMapStr( wchar_t * pStr );
void					VxUnMapStr( wchar_t * pStr );

