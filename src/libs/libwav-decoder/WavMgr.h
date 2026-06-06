#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <stdint.h>
#include <vector>
#include <string>

class WavMgr 
{
public:
	static bool readWavFile( std::string& fileName, std::vector<int16_t>& retBytes, int& retRate, int& retChannels, int& retBitsPerSample );
};
