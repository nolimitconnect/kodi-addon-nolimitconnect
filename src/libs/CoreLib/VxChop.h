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

#include <string>

//=== Chop utils for strings (saves a some bytes in length usually) ===//
int VxCalcChoppedLen( const char* pStr );
int VxGetUnchoppedStrLen( unsigned char * pu8ChopedStr );
int VxChopStr( const char* pStr, unsigned char * pu8RetChoppedStr );
int VxChopStr( std::string &csStr, unsigned char * pu8RetChoppedStr );
void VxUnchopStr(  unsigned char * pu8ChoppedStr, char * pRetUnchoppedStr );
void VxUnchopStr(  unsigned char * pu8ChoppedStr, std::string &csRetStr );

int VxCalcChoppedLen( const wchar_t * pStr );
int VxGetUnchoppedStrLen( uint16_t * pu8ChopedStr );
int VxChopStr( const wchar_t * pStr, uint16_t * pu8RetChoppedStr );
int VxChopStr( std::wstring &csStr, uint16_t * pu8RetChoppedStr );
void VxUnchopStr(  uint16_t * pu8ChoppedStr, wchar_t * pRetUnchoppedStr );
void VxUnchopStr(  uint16_t * pu8ChoppedStr, std::wstring &csRetStr );
