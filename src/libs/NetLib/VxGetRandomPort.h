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

//! get a random tcp/ip port that is not in use
//! if bAbove10000 is true then only consider ports above 10000
uint16_t VxGetRandomTcpPort( bool bAbove10000 = true ); 
