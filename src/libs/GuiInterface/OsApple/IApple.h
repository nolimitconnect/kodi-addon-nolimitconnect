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

class IApple : public OsInterface
{
public:
    IApple() = default;
    virtual ~IApple( ) = default;

    //=== stages of create ===//
    bool                        doPreStartup() override;
    bool                        doStartup() override;

    //=== stages of run ===//
    //bool                        initRun( const CAppParamParser &params ) override;
    //bool                        doRun() override;

    //=== stages of destroy ===//
    void                        doPreShutdown() override;
    void                        doShutdown() override;

    //=== utilities ===//
    bool                        initDirectories( ) override;

private:

};
