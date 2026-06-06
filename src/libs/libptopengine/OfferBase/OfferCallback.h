#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/VxSha1Hash.h>

class OfferBaseInfo;

class OfferCallback
{
public:
	virtual void				callbackFileWasShredded( std::string& fileName ){};

	virtual void				callbackHashIdGenerated( std::string& fileName, VxSha1Hash& hashId ){};
	virtual void				callbackOfferSendState( VxGUID& assetOfferId, EOfferSendState assetSendState, int param ){};
    virtual void				callbackOfferAction( VxGUID& assetOfferId, EOfferAction offerAction, int param ){};

	virtual void				callbackOfferFileTypesChanged( uint16_t fileTypes ){};
	virtual void				callbackOfferPktFileListUpdated( void ){};

    virtual void				callbackOfferAdded( OfferBaseInfo* assetInfo ){};
    virtual void				callbackOfferUpdated( OfferBaseInfo* assetInfo ){};
    virtual void				callbackOfferRemoved( VxGUID& offerId ){};
};

