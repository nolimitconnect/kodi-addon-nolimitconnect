#pragma once
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxSktBase.h"

// forward declare
class VxServerMgr;

class VxSktAccept : public VxSktBase
{
public:
	
	VxSktAccept();
	
	virtual ~VxSktAccept() = default;

	//! called when socket is accepted
    int32_t	doAccept( VxServerMgr * poMgr, struct sockaddr& oAcceptAddr );
};

