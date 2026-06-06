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

#include <GuiInterface/IDefs.h>

#include <CoreLib/VxGUID.h>

#include <string>

class AssetBaseInfo;
class MediaPlayerNlc;
class OsInterface;

class IMediaPlayerRequests
{
public:

    static MediaPlayerNlc&      getNlcPlayer( void );
    static OsInterface&         getOsInterface( void );

    virtual bool				fromThreadStartModule( EMediaModule mediaModule ) = 0;
    virtual bool				fromGuiStopModule( EMediaModule mediaModule ) = 0;
    virtual bool				fromGuiIsModuleRunning( EMediaModule mediaModule ) = 0;

    virtual bool				fromGuiPlayMedia( AssetBaseInfo& assetInfo, int pos0to100000 ) = 0;
    virtual bool				fromGuiMediaPlayerAction( EMediaPlayerAction playerAction ) = 0;
    virtual bool				fromGuiMediaPlayerSeek( int position0to100000 ) = 0;

    virtual bool				fromGuiPlayStream( AssetBaseInfo& assetInfo, VxGUID lclSessionId, int pos0to100000 ) = 0;

    virtual void				fromGuiGetCanSeek( void ) = 0;
    virtual void				fromGuiPlayPauseButtonClicked( void ) = 0;
    virtual void				fromGuiStopButtonClicked( void ) = 0;
    virtual void				fromGuiUpdatePlayPosition( void ) = 0;

    virtual void				fromGuiUpdateGlWidgetSize( int width, int height ) = 0;
};
