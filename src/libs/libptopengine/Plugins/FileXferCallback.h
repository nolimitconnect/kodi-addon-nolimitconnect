//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include <memory>

class VxPktHdr;
class VxSktBase;

class FileXferCallback
{
public:
	virtual void				onFileXferPktRxed( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr ) = 0;
};
