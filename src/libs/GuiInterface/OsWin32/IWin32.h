#pragma once
//============================================================================
// Copyright (C) 2018 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "GuiInterface/OsInterface/OsInterface.h"

#ifdef TARGET_OS_WINDOWS
#include "config_components_kodi.h"

class IWin32 : public OsInterface
{
public:
    IWin32() = default;
    virtual ~IWin32( ) = default;

    //=== stages of create ===//
    bool                        doPreStartup() override;
    bool                        doStartup() override;

    //=== stages of run ===//
    //bool                        initRun( const CAppParamParser &params ) override;
    //bool                        doRun() override;

    //=== stages of destroy ===//
    void                        doPreShutdown() override;
    void                        doShutdown() override;

private:

    HANDLE                       m_AppRunningMutex;
};

#endif // TARGET_OS_WINDOWS
