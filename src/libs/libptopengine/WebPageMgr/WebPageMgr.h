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

#include <CoreLib/VxMutex.h>

#include <string>
#include <map>
#include <vector>

class VxGUID;
class WebPageCallbackInterface;
class P2PEngine;

class WebPageMgr 
{
public:
	WebPageMgr( P2PEngine& engine );
	virtual ~WebPageMgr() = default;

    void                        fromGuiUserLoggedOn( void );

    void                        wantWebPageMgrCallbacks( WebPageCallbackInterface * client, bool enable );


    virtual void				announceWebDownloadStarted( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName, int fileNum );
    virtual void				announceWebDownloadProgress( EWebPageType webPageType, VxGUID& onlineId, int fileNum, int progress );
    virtual void				announceWebDownloadComplete( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName );
    virtual void				announceWebDownloadFailed( EWebPageType webPageType, VxGUID& onlineId, std::string& fileName, enum EXferError xferErr );


protected:
    void						lockClientList( void )						{ m_WebPageClientMutex.lock(); }
    void						unlockClientList( void )					{ m_WebPageClientMutex.unlock(); }

    P2PEngine&					m_Engine;
    VxMutex						m_ResourceMutex;
    bool						m_Initialized{ false };

    std::vector<WebPageCallbackInterface *> m_WebPageClients;
    VxMutex						m_WebPageClientMutex;
};

