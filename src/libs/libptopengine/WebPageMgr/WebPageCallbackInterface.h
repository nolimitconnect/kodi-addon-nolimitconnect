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
#include <CoreLib/VxXferDefs.h>

#include <string>

class VxGUID;

class WebPageCallbackInterface
{
public:
    virtual void				callbackWebDownloadStarted( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName, int fileNum ) {};
    virtual void				callbackWebDownloadProgress( EWebPageType webPageType, VxGUID& onlineId, int fileNum, int progress ) {};
    virtual void				callbackWebDownloadComplete( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName ) {};
    virtual void				callbackWebDownloadFailed( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName, enum EXferError xferErr ) {};
};

