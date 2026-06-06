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

#include <CoreLib/AssetDefs.h>

#include <memory>

class VxSktBase;
class VxGUID;

class AssetXferCallback
{
public:
	virtual void				callbackAssetXferReadyToSend( VxGUID& sendToId, std::shared_ptr<VxSktBase>& sktBase ) {};
    virtual void				callbackXferState( VxGUID& sendToId, VxGUID& assetId, enum EAssetSendState sendState, int param ) = 0;
};
