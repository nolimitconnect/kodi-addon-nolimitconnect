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

#define VxAbs(x) ((x)<0 ? -(x) : (x)) // get absolute value

#ifndef ROUND_TO_4BYTE_BOUNDRY
#define ROUND_TO_4BYTE_BOUNDRY( a ) (( a + 3 ) & ~3 ) //round upto even 4 byte boundry
#endif //ROUND_TO_4BYTE_BOUNDRY
#ifndef ROUND_TO_8BYTE_BOUNDRY
#define ROUND_TO_8BYTE_BOUNDRY( a ) (( a + 7 ) & ~7 ) //round upto even 8 byte boundry
#endif //ROUND_TO_8BYTE_BOUNDRY
#ifndef ROUND_TO_16BYTE_BOUNDRY
#define ROUND_TO_16BYTE_BOUNDRY( a ) (( a + 15 ) & ~15 ) //round upto even 16 byte boundry
#endif //ROUND_TO_16BYTE_BOUNDRY

