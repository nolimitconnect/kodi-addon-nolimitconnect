//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#ifndef VXLINUXONLY_H_INCLUDED
#define VXLINUXONLY_H_INCLUDED

#ifndef TARGET_OS_WINDOWS

//=== functions missing in linux that are in visual studio ===//
//! convert int to ascii string
char * itoa(int value, char* str, int base);
//! convert long to ascii string
char * ltoa(long value, char* str, int base);
//! convert unsigned long to ascii string
char * ultoa(unsigned long value, char* str, int base);
//! convert string to upper case
void strupr( char * pStr );
//! convert string to lower case
void strlwr( char * pStr );

#ifndef TARGET_OS_ANDROID
//! case insensitive string compare
int stricmp( const char* pStr1, const char* pStr2 );
//! case insensitive string compare
int strnicmp( const char* pStr1, const char* pStr2, int n );
#endif // TARGET_OS_ANDROID

#endif // TARGET_OS_WINDOWS
#endif // VXLINUXONLY_H_INCLUDED
