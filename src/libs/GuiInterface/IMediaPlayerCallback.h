#pragma once
//============================================================================
// Copyright (C) 2023 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

class VxGUID;
class MediaPlayerNlc;

class IMediaPlayerCallback
{
public:

    virtual void				fromMediaPlayerInitLevel( int initLevel, bool success ) = 0;
    virtual void				fromMediaPlayerIsRunning( bool isRunning ) = 0;

    virtual void				fromMediaPlayerPlayFile( VxGUID& feedId, bool fileOpened ) = 0;

    virtual void				fromMediaPlayerPlaybackStarted( VxGUID& feedId ) = 0;
    virtual void				fromMediaPlayerPlaybackStopped( VxGUID& feedId ) = 0;

    virtual void				fromMediaPlayerPlaybackEnded( VxGUID& feedId ) = 0;
    virtual void				fromMediaPlayerPlayPause( VxGUID& feedId, bool isPaused ) = 0;

    virtual void				fromMediaPlayerCanSeek( VxGUID& feedId, bool canSeek, bool canPause ) = 0;
    virtual void				fromMediaPlayerUpdatePlayPosition( VxGUID& feedId, int position0to100000 ) = 0;
};

