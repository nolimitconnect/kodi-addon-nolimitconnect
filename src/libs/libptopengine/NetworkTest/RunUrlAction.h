#pragma once
//============================================================================
// Copyright (C) 2020 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "NetworkTestBase.h"

#include <CoreLib/VxUrl.h>

class UrlActionInfo;
class IConnectRequestCallback;

class UrlActionResultInterface
{
public:
    virtual void                callbackActionStatus( UrlActionInfo& actionInfo, enum ERunTestStatus eStatus, enum ENetCmdError netCmdError, std::string statusMsg ) = 0;
    virtual void                callbackActionFailed( UrlActionInfo& actionInfo, enum ERunTestStatus eStatus, enum ENetCmdError netCmdError ) = 0;

    virtual void                callbackPingSuccess( UrlActionInfo& actionInfo, std::string myIp ) = 0;
    virtual void                callbackConnectionTestSuccess( UrlActionInfo& actionInfo, bool canDirectConnect, std::string myIp ) = 0;
    virtual void                callbackQueryIdSuccess( UrlActionInfo& actionInfo, VxGUID onlineId ) = 0;
};

class UrlActionInfo
{
public:
    UrlActionInfo();
    UrlActionInfo(P2PEngine& engine, enum EHostType hostType, VxGUID& sessionId, enum ENetCmdType testType, const char* ptopUrl, const char* myUrl, 
                  UrlActionResultInterface* cbInterface, IConnectRequestCallback* cbConnectReq, enum EConnectReason connectReason );
    UrlActionInfo( const UrlActionInfo& rhs );
    UrlActionInfo&              operator = ( const UrlActionInfo& rhs );
    bool                        operator == ( const UrlActionInfo& rhs ) const;
    bool                        operator != ( const UrlActionInfo& rhs ) const;

    P2PEngine&					getEngine( void )                   { return m_Engine; }
    enum EHostType              getHostType( void )                 { return m_HostType; }
    VxUrl&                      getMyVxUrl( void )                  { return m_MyUrl; }
    std::string&                getMyUrl( void )                    { return m_MyUrl.getUrl(); }
    enum ENetCmdType            getNetCmdType( void )               { return m_TestType; }
    VxUrl&                      getRemoteVxUrl( void )              { return m_RemoteUrl; }
    std::string&                getRemoteUrl( void )                { return m_RemoteUrl.getUrl(); }
    UrlActionResultInterface*   getResultInterface( void )          { return m_ResultCbInterface; }
    IConnectRequestCallback*    getConnectReqInterface( void )      { return m_ConnectReqCbInterface; }
    enum EConnectReason         getConnectReason( void )            { return m_ConnectReason; };
    void                        setSessionId( VxGUID& sessionId )   { m_SessionId = sessionId; };
    VxGUID&                     getSessionId( void )                { return m_SessionId; };

    std::string                 getTestName( void );

protected:
    //=== vars ===//
    P2PEngine&					m_Engine;
    enum EHostType              m_HostType{ eHostTypeUnknown };
    UrlActionResultInterface*   m_ResultCbInterface{ nullptr };
    IConnectRequestCallback*    m_ConnectReqCbInterface{ nullptr };
    enum ENetCmdType            m_TestType{ eNetCmdUnknown };
    VxUrl                       m_MyUrl;
    VxUrl                       m_RemoteUrl;
    enum EConnectReason         m_ConnectReason{ eConnectReasonUnknown };
    VxGUID                      m_SessionId;
};


class RunUrlAction
{
public:
    RunUrlAction( P2PEngine& engine, EngineSettings& engineSettings, NetServicesMgr& netServicesMgr, NetServiceUtils& netServiceUtils );
	virtual ~RunUrlAction() = default;
    RunUrlAction() = delete; // don't allow default constructor
    RunUrlAction(const RunUrlAction&) = delete; // don't allow copy constructor

    P2PEngine&					getEngine() { return m_Engine; }

	void				        runUrlAction( VxGUID& sessionId,
                                              enum ENetCmdType netCmdType, 
                                              const char* ptopUrl, 
                                              const char* myUrl = nullptr,                                        
                                              UrlActionResultInterface* cbInterface = nullptr,
                                              IConnectRequestCallback* cbConnectRequest = nullptr, 
                                              enum EHostType hostType = eHostTypeUnknown,
                                              enum EConnectReason connectReason = eConnectReasonUnknown );
	void						runTestShutdown( void );

	void						threadFuncRunUrlAction( void );

protected:
    void                        startUrlActionThread( void );
    bool                        isThreadRunningActions( void );
    ERunTestStatus			    doUrlAction( UrlActionInfo& urlInfo );
    ERunTestStatus			    doRunTestFailed( UrlActionInfo& urlAction, std::string& urlActionName, enum ERunTestStatus eTestStatus, enum ENetCmdError cmdErr );
    ERunTestStatus			    doRunTestSuccess( UrlActionInfo& urlAction, std::string& urlActionName );

    void						sendRunTestStatus( UrlActionInfo& urlAction, std::string& urlActionName, enum ERunTestStatus eTestStatus, enum ENetCmdError cmdErr, const char* msg, ... );
    void						sendTestLog( UrlActionInfo& urlAction, std::string& urlActionName, const char* msg, ... );

    void                        hostQueryIdConnectFailed( UrlActionInfo& urlInfo );

    //=== vars ===//
    P2PEngine&					m_Engine;
    EngineSettings&				m_EngineSettings;
    NetServicesMgr&				m_NetServicesMgr;
    NetServiceUtils&			m_NetServiceUtils;

    std::vector<UrlActionInfo>  m_UrlActionList;
    VxMutex                     m_ActionListMutex;
    bool                        m_ThreadIsRunningActions{ false };

    VxThread					m_RunActionThread;
};

