
#pragma once

#include "Application.h"

#include "FileItem.h"

#include <GuiInterface/IMediaPlayerRequests.h>
#include <GuiInterface/IDefs.h>

#include <AssetMgr/AssetInfo.h>

#include <CoreLib/VxMutex.h>

class IMediaPlayerCallback;

class MediaPlayerNlc : public CApplication, public IMediaPlayerRequests
{
public:
	MediaPlayerNlc();

	IMediaPlayerRequests&		getIMediaPlayerRequests( void );

	void						wantMediaPlayerCallback( IMediaPlayerCallback* client, bool wantCallback );

	enum EMediaModule			getMediaModule( void ) override;
    bool						testQuitFlag() override;

	void						setIsPlayingMedia( bool isPlaying ) { m_IsPlayingMedia = isPlaying; }
	bool						getIsPlayingMedia( void ) override { return m_IsPlayingMedia; }
	void						setIsPlayingVideo( bool isPlaying ) { m_IsPlayingVideo = isPlaying; }
	bool						getIsPlayingVideo( void ) override { return m_IsPlayingVideo; }

	bool						fromThreadStartModule( EMediaModule mediaModule ) override;
	bool						fromGuiStopModule( EMediaModule mediaModule ) override;
	bool						fromGuiIsModuleRunning( EMediaModule mediaModule ) override;

	bool						fromGuiPlayMedia( AssetBaseInfo& assetInfo, int pos0to100000 ) override;
	bool						fromGuiMediaPlayerAction( EMediaPlayerAction playerAction ) override;
	bool						fromGuiMediaPlayerSeek( int position0to100000 ) override;

	bool						fromGuiPlayStream( AssetBaseInfo& assetInfo, VxGUID lclSessionId, int pos0to100000 ) override;

	void						fromGuiGetCanSeek( void ) override;
    void						fromGuiPlayPauseButtonClicked( void ) override;
    void						fromGuiStopButtonClicked( void ) override;
	void						fromGuiUpdatePlayPosition( void ) override;

	void						fromGuiUpdateGlWidgetSize( int width, int height ) override;

	void						fromGuiAppShutdown( void );

protected:
    bool						assureInitialized( void );

	bool						playAudioFile( int position0to100000 );
	bool						playVideoFile( int position0to100000 );

	void						onInitLevel( int initLevel, bool success ) override;
	void						onPlayerRunning( bool isRunning ) override;

	void						onPlayFile( bool fileOpened ) override;
    void						onPlayStarted( void ) override;
    void						onStopPlaying( void ) override;

    void						onPlaybackPaused( void ) override;
    void						onPlaybackResumed( void ) override;
    void						onPlaybackError( void ) override;
    void						onPlaybackStopped( void )  override;
    void						onPlaybackEnded( void ) override;

	void						onPlayPause( bool isPaused ) override;
	void						onCanSeek( bool canSeek, bool canPause ) override;

	void						lockClientList( void ) { m_MediaPlayerCallbackMutex.lock(); }
	void						unlockClientList( void ) { m_MediaPlayerCallbackMutex.unlock(); }

	void						initVideoSettings( void );

	//=== vars ===//
	//static CAppParamParser		m_AppParamParser;
	AssetBaseInfo				m_AssetInfo;
	CFileItem					m_FileItem;

	bool						m_ModuleIsInitialized{ false };
	bool						m_ModuleIsRunning{ false };
	bool						m_ModuleStopCalled{ false };
	
	bool						m_IsPlayingMedia{ false };
	bool						m_IsPlayingVideo{ false };

	std::vector<IMediaPlayerCallback*>    m_MediaPlayerCallbackClients;
    VxMutex						m_MediaPlayerCallbackMutex;

	VxGUID						m_FeedId;
	VxGUID						m_SessionId;
};

MediaPlayerNlc& GetNlcPlayerInstance();
