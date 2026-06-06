//============================================================================
// Copyright (C) 2023 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include <stddef.h>

void* VxAlignedMalloc( size_t memSize, size_t alignBitCount = 64 );

void VxAlignedFree( void* alignMemPtr, bool glAllocated = false );
