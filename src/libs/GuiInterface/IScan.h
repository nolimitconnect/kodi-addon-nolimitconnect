//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include <GuiInterface/IDefs.h>

#include <memory>

class VxSktBase;
class VxNetIdent;

//! IScan is an abstract interface for scan/search results
class IScan
{
public:

	//! error occurred during scan/search
	virtual void	onScanResultError(	EScanType			eScanType,
										VxNetIdent*			netIdent,
										std::shared_ptr<VxSktBase>&			sktBase,  
										uint32_t			errorCode ) = 0; 

	//! About Me Web Page scan pictures success result
	virtual void	onScanResultProfilePic(	VxNetIdent*		netIdent, 
											std::shared_ptr<VxSktBase>&		sktBase, 
											uint8_t *		pu8JpgData, 
											uint32_t		u32JpgDataLen ) = 0;
};
