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

#include "IDefs.h"

#define ILOG_LEVEL_NONE         (0) // none ( disable )
#define ILOG_LEVEL_INFO	        (1) // info only
#define ILOG_LEVEL_FATAL	    (2) // fatal + info
#define ILOG_LEVEL_ASSERT	    (3) // assert + fatal + info
#define ILOG_LEVEL_SEVERE	    (4) // severe + assert + fatal + info
#define ILOG_LEVEL_ERROR	    (5) // error + severe + assert + fatal + info
#define ILOG_LEVEL_WARN	        (6) // warn + error + severe + assert + fatal + info
#define ILOG_LEVEL_DEBUG	    (7) // debug + warn + error + severe + assert + fatal + info
#define ILOG_LEVEL_VERBOSE      (8) // all

#define ILOG_LEVEL_RELEASE ILOG_LEVEL_ERROR // change this to change what default log level is when building release

class INlc;

class ILog
{
public:
    ILog( );
    virtual  ~ILog() = default;

    // call these first to set all log levels
    void                        setAllLogLevel( int level );
    void                        clearAllLogLevel();

    // adjust log levels of different modules after the "all" level is set
    // this level is overall end log output.. levels of others higher than this will still be filtered out
    void                        setCoreLogLevel( int level );

    void                        setKodiLogLevel( int level );
    void                        clearKodiLogLevel();

    void                        setFfmpegLogLevel( int level );
    int                         getFfmpegLogLevel() { return m_ffmpegLogLevel; }
    void                        clearFfmpegLogLevel();

private:
    int                         m_CoreLogLevel{ 0 };
    int                         m_ffmpegLogLevel{ 0 };
    int                         m_kodiLogLevel{ 0 };
};
