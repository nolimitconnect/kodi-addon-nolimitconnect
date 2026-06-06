#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "FileShareXferSession.h"

class FileRxSession : public FileShareXferSession
{
public:
	FileRxSession();
	FileRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	FileRxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID& sendToId );
	virtual ~FileRxSession() = default;

	void cancelDownload( VxGUID& lclSessionId );
};
