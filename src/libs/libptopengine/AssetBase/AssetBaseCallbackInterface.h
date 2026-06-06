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



#include <CoreLib/AssetDefs.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/VxSha1Hash.h>

class AssetBaseInfo;

class AssetBaseCallbackInterface
{
public:
	virtual void				callbackFileWasShredded( std::string& fileName ){};

	virtual void				callbackHashIdGenerated( std::string& fileName, VxSha1Hash& hashId ){};
	virtual void				callbackAssetSendState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState assetSendState, int param ){};

	virtual void				callbackAssetFileTypesChanged( uint16_t fileTypes ){};
	virtual void				callbackAssetPktFileListUpdated( void ){};

    virtual void				callbackAssetAdded( AssetBaseInfo* assetInfo ){};
    virtual void				callbackAssetUpdated( AssetBaseInfo* assetInfo ){};
    virtual void				callbackAssetRemoved( AssetBaseInfo* assetInfo ){};
    virtual void				callbackAssetHistory( void * userData, AssetBaseInfo* assetInfo ){};
};

