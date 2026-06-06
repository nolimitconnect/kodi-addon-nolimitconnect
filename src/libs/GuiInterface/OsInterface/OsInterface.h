#pragma once
//============================================================================
// Copyright (C) 2023 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "GuiInterface/IDefs.h"
#include "GuiInterface/INlcEvents.h"

#include <string>

class CAppParamParser;

class OsInterface
{
public:
    OsInterface() = default;
    virtual ~OsInterface() = default;

    // exit of application error code
    virtual void                setRunResultCode( int exitCode )    { m_RunResultCode = exitCode; }
    virtual int                 getRunResultCode( void )            { return m_RunResultCode; }

    //=== stages of create ===//
    virtual bool                doPreStartup( void ) = 0;
    virtual bool                doStartup( void ) = 0;

    //=== stages of run ===//
    virtual bool                doRun( EMediaModule mediaModule );

    //=== stages of destroy ===//
    virtual void                doPreShutdown( void ) = 0;
    virtual void                doShutdown( void ) = 0;

    //=== utilities ===//
    virtual bool               initUserPaths( std::string& userWriteablePath ); // basically exe and user data paths
    virtual bool               initDirectories( void );

protected:
    int                         m_RunResultCode = 0;
};
