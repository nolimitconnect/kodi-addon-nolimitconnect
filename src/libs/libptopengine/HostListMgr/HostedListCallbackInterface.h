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

class HostedInfo;
class VxGUID;

class HostedListCallbackInterface
{
public:
    virtual void				callbackHostedInfoListUpdated( HostedInfo* hostedInfo ){};
    virtual void				callbackHostedInfoListRemoved( VxGUID& userOnlineId, enum EHostType hostType ){};
    virtual void				callbackHostedInfoListSearchResult( HostedInfo* hostedInfo, VxGUID& sessionId ) {};
    virtual void				callbackHostedInfoListSearchStatus( enum EHostType hostType, VxGUID& sessionId, enum EConnectStatus connectStatus ) {};
    virtual void				callbackHostedInfoListSearchComplete( enum EHostType hostType, VxGUID& sessionId ) {};
};

