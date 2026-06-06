#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

class Sha1Info;
class VxGUID;

class Sha1GeneratorCallback
{
public:
	virtual void				callbackSha1GenerateResult( ESha1GenResult sha1GenResult, VxGUID& fileId, Sha1Info& sha1Info ) = 0;
};
