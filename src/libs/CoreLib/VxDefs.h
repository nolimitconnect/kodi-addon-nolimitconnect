#pragma once

//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#define VX_DEFS 1

#include <CoreLib/config_corelib.h>

// to avoid multiple defines and missing defines
//#define VXMIN(_a,_b)      ((_a)<(_b)?(_a):(_b))
//#define VXMAX(_a,_b)      ((_a)>(_b)?(_a):(_b))
#define VXCLAMP(_a,_b,_c) (VXMAX(_a,VXMIN(_b,_c)))

#define S16_MAXVAL		32767	
#define S16_MINVAL		-32768	
#define U16_MAXVAL		65535	
#define U16_MINVAL		0

#define S32_MAXVAL		2147483647	
#define S32_MINVAL		-2147483648	
#define U32_MAXVAL		4294967295	
#define U32_MINVAL		0	
#define S64_MAXVAL		9223372036854775807
#define S64_MINVAL		-9223372036854775808
#define U64_MAXVAL		18446744073709551615	
#define U64_MINVAL		0	


