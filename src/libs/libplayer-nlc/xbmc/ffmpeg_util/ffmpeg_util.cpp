//============================================================================
// Copyright (C) 2023 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxDebug.h>

extern "C" {
#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxMutex.h>
#include <CoreLib/VxStringUtils.h>

//============================================================================
void ff_flush_avutil_log_buffers(void)
{
}

//============================================================================
void ff_avutil_log(void* ptr, int level, const char* format, va_list va)
{
    static VxMutex logMutex;
    logMutex.lock();


    AVClass* avc= ptr ? *(AVClass**)ptr : NULL;

    int logLevel = level;
    int logType = 0;
    switch( level )
    {
    case AV_LOG_FATAL:
    case AV_LOG_PANIC:
        logType = LOG_FATAL;
        break;

    case AV_LOG_ERROR:
        logType = LOG_ERROR;
        break;

    case AV_LOG_WARNING:
        logType = LOG_WARN;
        break;

    case AV_LOG_INFO:
        logType = LOG_INFO;
        break;

    case AV_LOG_DEBUG:
        logType = LOG_DEBUG;
        break;

    case AV_LOG_TRACE:
    case AV_LOG_VERBOSE:
        logType = LOG_VERBOSE;
        break;

    case AV_LOG_QUIET:
    default:
        logMutex.unlock();
        return;
    }

    if( !LogEnabled( eLogFfmpeg ) )
    {
        logMutex.unlock();
        return;
    }

    /*
    if( logLevel > GetILog().getFfmpegLogLevel() )
    {
        logMutex.unlock();
        return;
    }
    */

    if( level > AV_LOG_VERBOSE ) // AV_LOG_DEBUG && AV_LOG_TRACE is so verbose to the point of not really able to play 
    {
        logMutex.unlock();
        return;
    }

    std::string message = VxStringUtils::FormatV( format, va );
    if( message.empty() )
    {
        logMutex.unlock();
        return;
    }

    static std::string buffer;
    std::string prefix = "ffmpeg: ";
    if( avc )
    {
        if( avc->item_name )
            prefix += std::string( "[" ) + avc->item_name( ptr ) + "] ";
        else if( avc->class_name )
            prefix += std::string( "[" ) + avc->class_name + "] ";
    }

    bool logged{ false };
    buffer += message;
    int pos, start = 0;
    while( buffer.length() && ( pos = (int)buffer.find_first_of( '\n', start ) ) >= 0 )
    {
        if( pos > start )
        {
            std::string msgStr = buffer.substr( start, pos - start ).c_str();
            if( buffer.length() && msgStr.length() )
            {
                LogModule( eLogFfmpeg, logType, "%s%s", prefix.c_str(), msgStr.c_str() );
                logged = true;
            }
        }

        start = pos + 1;
    }

    if( !logged && buffer.length() )
    {
        // show duration stats
        LogModule( eLogFfmpeg, logType, "ffmpeg: %s", buffer.c_str() );
    }

    buffer.erase( 0, start );
    logMutex.unlock();
}

//=== ffmpeg ===//
//============================================================================
void startupFfmpeg()
{
    LogModule( eLogStartup, LOG_VERBOSE, "startupFfmpeg" );
    static bool ffmpegStarted = false;
    if( !ffmpegStarted )
    {
        ffmpegStarted = true;
        // register ffmpeg lockmanager callback
    //    av_lockmgr_register( &ffmpeg_lockmgr_cb );

        // set avutil callback
        //av_log_set_callback( ff_avutil_log );
        // register avcodec
        // BRJ avcodec_register_all();
#if CONFIG_AVDEVICE
 //       avdevice_register_all();
#endif
        // register avformat
        // BRJ av_register_all();
        // register avfilter
        // BRJ avfilter_register_all();
        // initialize network protocols
        //avformat_network_init();
    }
}

//============================================================================
void shutdownFfmpeg()
{

}