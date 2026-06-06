/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "config_components_kodi.h"

#include <GuiInterface/INlcEvents.h>
#include <GuiInterface/IDefs.h>
#include <GuiInterface/IMediaPlayerRequests.h>

#include "ApplicationComponents.h"
#include "ApplicationEnums.h"
#include "ApplicationPlayerCallback.h"

#include "guilib/IMsgTargetCallback.h"
#include "guilib/IWindowManagerCallback.h"
#include "messaging/IMessageTarget.h"
#include "playlists/PlayListTypes.h"
#include "threads/SystemClock.h"
#include "threads/Thread.h"
#include "utils/GlobalsHandling.h"
#include "utils/Stopwatch.h"
#include "windowing/Resolution.h"
#include "windowing/XBMC_events.h"

#include <atomic>
#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <vector>

class CAction;
class CAppInboundProtocol;
class CBookmark;
class CFileItem;
class CFileItemList;
class CGUIComponent;

class CKey;
class CSeekHandler;
class CServiceManager;
class CSettingsComponent;
class CSplash;
class CWinSystemBase;

class MediaPlayerNlc;

namespace ADDON
{
    class CSkinInfo;
    class IAddon;
    typedef std::shared_ptr<IAddon> AddonPtr;
    class CAddonInfo;
}

namespace ANNOUNCEMENT
{
    class CAnnouncementManager;
}

namespace MEDIA_DETECT
{
    class CAutorun;
}

namespace PLAYLIST
{
    class CPlayList;
}

namespace ActiveAE
{
    class CActiveAE;
}

class CApplication : public IWindowManagerCallback,
    public IMsgTargetCallback,
    public KODI::MESSAGING::IMessageTarget,
    public CApplicationComponents,
    public CApplicationPlayerCallback,
//    public CApplicationSettingsHandling,
    public INlcEvents
{
    friend class CAppInboundProtocol;

public:

    // If playback time of current item is greater than this value, ACTION_PREV_ITEM will seek to start
    // of currently playing item, otherwise it will seek to start of the previous item in playlist
    static const unsigned int ACTION_PREV_ITEM_THRESHOLD = 3; // seconds;

    CApplication( void );
    ~CApplication( void ) override;


    virtual enum EMediaModule   getMediaModule( void ) { return eMediaModuleMediaPlayer; }
    virtual bool                testQuitFlag() { return false; }

    virtual void                setRenderGui(bool renderGui) { m_RenderGui = renderGui; }
    virtual bool                getRenderGui() { return m_RenderGui; }

    virtual void                onPlayerRunning( bool isRunning ) = 0;

    virtual bool                getIsPlayingMedia( void ) = 0;
    virtual bool                getIsPlayingVideo( void ) = 0;

    virtual void                setRenderWindowSize( int winWidth, int winHeight );
    virtual void                getRenderWindowSize( int& winWidth, int& winHeight );
    virtual void                onRenderWindowResized( int winWidth, int winHeight ) {};

    //============================================================================
    //=== nlc events ===//
    //============================================================================
    void                        fromGuiKeyPressEvent( EMediaModule mediaModule, int key, int mod ) override;
    void                        fromGuiKeyReleaseEvent( EMediaModule mediaModule, int key, int mod ) override;

    void                        fromGuiMousePressEvent( EMediaModule mediaModule, int mouseXPos, int mouseYPos, int mouseButton ) override;
    void                        fromGuiMouseReleaseEvent( EMediaModule mediaModule, int mouseXPos, int mouseYPos, int mouseButton ) override;
    void                        fromGuiMouseMoveEvent( EMediaModule mediaModule, int mouseXPos, int mouseYPos ) override;

    void                        fromGuiRenderWindowResize( EMediaModule mediaModule, int winWidth, int winHeight ); // override;

    void                        fromGuiCloseEvent( EMediaModule mediaModule ) override;
    void                        fromGuiVisibleEvent( EMediaModule mediaModule, bool isVisible ) override;

    bool                        toGuiMediaAction( EMediaPlayerAction playerAction, int actionVal, const char* fileName );
    void                        toGuiMediaError( EMediaError mediaError, const char* msg );
    
    virtual void				onInitLevel( int initLevel, bool success ) {};

    virtual void                onPlayFile( bool fileOpened ) {};
    virtual void                onPlayStarted( void ) {};
    virtual void                onPlaybackPaused( void ) {};
    virtual void                onPlaybackResumed( void ) {};
    virtual void                onPlaybackError( void ) {};

    virtual void                onStopPlaying( void ) {};
    virtual void                onPlaybackStopped( void ) {};
    virtual void                onPlaybackEnded( void ) {};
    virtual void                onPlayPause( bool isPaused ) {};
    virtual void                onCanSeek( bool canSeek, bool canPause ) {};

    void PlayPauseButtonClicked( void );

    //============================================================================

    bool Create();
    bool Initialize();
    int Run();
    bool Cleanup();

    void FrameMove( bool processEvents, bool processGUI = true ) override;
    void Render() override;

    bool IsInitialized() const { return !m_bInitializing; }
    bool IsStopping() const { return m_bStop; }

    bool CreateGUI();
    bool InitWindow( RESOLUTION res = RES_INVALID );

    bool Stop( int exitCode );
    const std::string& CurrentFile();
    CFileItem& CurrentFileItem();
    std::shared_ptr<CFileItem> CurrentFileItemPtr();
    const CFileItem& CurrentUnstackedItem();
    bool OnMessage( CGUIMessage& message ) override;
    std::string GetCurrentPlayer();

    int  GetMessageMask() override;
    void OnApplicationMessage( KODI::MESSAGING::ThreadMessage* pMsg ) override;

    bool PlayMedia( CFileItem& item, const std::string& player, PLAYLIST::Id playlistId );
    bool ProcessAndStartPlaylist( const std::string& strPlayList,
                                  PLAYLIST::CPlayList& playlist,
                                  PLAYLIST::Id playlistId,
                                  int track = 0 );
    bool PlayFile( CFileItem item, const std::string& player, bool bRestart = false );
    void StopPlaying();
    void Restart( bool bSamePosition = true );
    void DelayedPlayerRestart();
    void CheckDelayedPlayerRestart();
    bool IsPlayingFullScreenVideo() const;
    bool IsFullScreen();
    bool OnAction( const CAction& action );
    void CloseNetworkShares();

    void ConfigureAndEnableAddons();
    void ShowAppMigrationMessage();
    void Process() override;
    void ProcessSlow();
    /*!
     \brief Returns the total time in fractional seconds of the currently playing media

     Beware that this method returns fractional seconds whereas IPlayer::GetTotalTime() returns milliseconds.
     */
    double GetTotalTime() const;
    /*!
     \brief Returns the current time in fractional seconds of the currently playing media

     Beware that this method returns fractional seconds whereas IPlayer::GetTime() returns milliseconds.
     */
    double GetTime() const;
    float GetPercentage() const;

    // Get the percentage of data currently cached/buffered (aq/vq + FileCache) from the input stream if applicable.
    float GetCachePercentage() const;

    void SeekPercentage( float percent );
    void SeekTime( double dTime = 0.0 );

    void UpdateCurrentPlayArt();


    std::string m_strPlayListFile;

    bool IsAppFocused() const { return m_AppFocused; }

    bool GetRenderGUI() const override;

    bool SetLanguage( const std::string& strLanguage );
    bool LoadLanguage( bool reload );

    void SetLoggingIn( bool switchingProfiles );

    std::unique_ptr<CServiceManager> m_ServiceManager;

    /*!
    \brief Locks calls from outside kodi (e.g. python) until framemove is processed.
    */
    void LockFrameMoveGuard();

    /*!
    \brief Unlocks calls from outside kodi (e.g. python).
    */
    void UnlockFrameMoveGuard();

    bool SwitchToFullScreen( bool fullScreen );

    void WakeUpScreenSaverAndDPMS( void ) {}; // gained focus.. what to do

protected:
    void PlaybackCleanup();

    // inbound protocol
    bool OnEvent( XBMC_Event& newEvent );

    std::shared_ptr<ANNOUNCEMENT::CAnnouncementManager> m_pAnnouncementManager;
    std::unique_ptr<CGUIComponent> m_pGUI;
    std::unique_ptr<CWinSystemBase> m_pWinSystem;
    std::unique_ptr<ActiveAE::CActiveAE> m_pActiveAE;
    std::shared_ptr<CAppInboundProtocol> m_pAppPort;
    std::deque<XBMC_Event> m_portEvents;
    CCriticalSection m_portSection;

    // timer information
    CStopWatch m_restartPlayerTimer;
    CStopWatch m_frameTime;
    CStopWatch m_slowTimer;
    XbmcThreads::EndTime<> m_guiRefreshTimer;

    std::string m_prevMedia;
    bool m_bInitializing = true;

    int m_nextPlaylistItem = -1;

    std::chrono::time_point<std::chrono::steady_clock> m_lastRenderTime;
    bool m_skipGuiRender = false;

    bool PlayStack( CFileItem& item, bool bRestart );

    void HandlePortEvents();


public:
    virtual void                setRenderThreadId( ThreadIdentifierKodi guiThreadId )
    {
        m_threadID = guiThreadId;
    }

    bool                        IsCurrentThread() const;

    ThreadIdentifierKodi        m_threadID{ 0 };
    bool                        m_bStop{false};
    bool                        m_AppFocused{ true };

    bool                        m_RenderGui{ true };

protected:
    int                         m_RenderWindowWidth{ 0 };
    int                         m_RenderWindowHeight{ 0 };
    mutable CCriticalSection    m_critical;
    bool                        m_RenderSystemInitialized{ false };

private:
    void PrintStartupLog();
    void ResetCurrentItem();

    mutable CCriticalSection m_critSection; /*!< critical section for all changes to this class, except for changes to triggers */

    CCriticalSection m_frameMoveGuard;              /*!< critical section for synchronizing GUI actions from inside and outside (python) */
    std::atomic_uint m_WaitingExternalCalls;        /*!< counts threads which are waiting to be processed in FrameMove */
    unsigned int m_ProcessedExternalCalls = 0;      /*!< counts calls which are processed during one "door open" cycle in FrameMove */
    unsigned int m_ProcessedExternalDecay = 0;      /*!< counts to close door after a few frames of no python activity */
    int m_ExitCode{ EXITCODE_QUIT };
};


CApplication& GetKodiInstance();

#define g_application GetKodiInstance()
