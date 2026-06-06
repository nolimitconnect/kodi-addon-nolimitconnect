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

class ThumbInfo;

class ThumbCallbackInterface : public AssetBaseCallbackInterface
{
public:
    virtual void				callbackThumbAdded( ThumbInfo* thumbInfo ){};
    virtual void				callbackThumbUpdated( ThumbInfo* thumbInfo ){};
    virtual void				callbackThumbRemoved( VxGUID& thumbId ){};
};

