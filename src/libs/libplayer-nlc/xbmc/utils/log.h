/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>
#include <utility>

#include "commons/ilog.h"
#include "utils/logtypes.h"
#include "utils/StringUtils.h"


class CLog
{
public:
    CLog();
    ~CLog();
    static void Close();

    // too many issues with templates so only old school style logging

    static void Log( int loglevel, const char* format, ... );

    static void Log( int loglevel, int component, const char* format, ... );

    static void LogF( int loglevel, const char* format, ... );

    static void LogFC( int loglevel, int component, const char* format, ... );

    static void LogFunction( int loglevel, std::string functionName, const char* format )
    {
        if( IsLogLevelLogged( loglevel ) )
            LogString( loglevel, functionName + ": " + format );
    }

    static void LogFunction( int loglevel, std::string functionName, int component, const char* format )
    {
        if( IsLogLevelLogged( loglevel ) )
            LogString( loglevel, component, functionName + ": " + format );
    }


    static void MemDump( char* pData, int length );
    static bool Init( const std::string& path );
    static void PrintDebugString( const std::string& line ); // universal interface for printing debug strings
    static void SetLogLevel( int level );
    static int  GetLogLevel();
    static void SetExtraLogLevels( int level );
    static bool IsLogLevelLogged( int loglevel );

    /// @brief convert kodi log level to mask used by NLC in VxDebug.h
    static int kodiToNlcLogLevel( int logLevel );

    Logger GetLogger( const std::string& loggerName );

    bool CanLogComponent( int component /*not used*/)
    {
        return true;
    }

    static CLog& GetInstance();

protected:
    static void LogString( int logLevel, std::string&& logString );
    static void LogString( int logLevel, int component, std::string&& logString );
    static bool WriteLogString( int logLevel, const std::string& logString );
};
