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

#include <AssetBase/AssetBaseCallbackInterface.h>

class BlobInfo;

class BlobCallbackInterface : public AssetBaseCallbackInterface
{
public:
	virtual void				callbackBlobAdded( BlobInfo* blobInfo ){};
	virtual void				callbackBlobRemoved( BlobInfo* blobInfo ){};
    virtual void				callbackBlobSendState( VxGUID& assetUniqueId, enum EAssetSendState assetSendState, int param ){};

	virtual void				callbackBlobHistory( BlobInfo* blobInfo ){};
};

