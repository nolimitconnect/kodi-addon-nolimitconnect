#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "CamJpgVideo.h"
#include "AudioPcmData.h"

#include <GuiInterface/IFromGui.h>
#include <GuiInterface/IAudioDefs.h>

#include <PktLib/PktVoiceReq.h>

#include <CoreLib/VxMutex.h>
#include <CoreLib/VxThread.h>
#include <CoreLib/VxSemaphore.h>

#include <ffmpeg/opus/OpusCodec.h>

#include <memory>
#include <vector>

enum EMediaInputCategory
{
	eMediaInputCategoryUnknown,
	eMediaInputCategoryMixer,
	eMediaInputCategoryAudio,
	eMediaInputCategoryVideo,

	eMaxMediaInputCategory
};

class PluginBase;
class IToGui;
class MediaTools;
class MediaClient;
class PktVideoFeedPic;
class PktVideoFeedPicChunk;
class P2PEngine;
class PluginBase;
class PluginMgr;

class ClientToRemove
{
public:
	ClientToRemove();
	ClientToRemove( VxGUID&						onlineId,
					EMediaInputType				mediaType, 
					MediaCallbackInterface *	callback,
					EMediaModule				mediaModule,
					VxGUID&						sessionId)
	: m_OnlineId( onlineId )
	, m_MediaType( mediaType )
	, m_Callback( callback )
	, m_MediaModule( mediaModule )
	, m_SessionId( sessionId )
	{
	}

	ClientToRemove( const ClientToRemove& rhs )
	{
		if( &rhs != this )
		{
			*this = rhs;
		}
	}

	ClientToRemove&				operator =( const ClientToRemove& rhs )
	{
		if( &rhs != this )
		{
			m_OnlineId = rhs.m_OnlineId;
			m_MediaType = rhs.m_MediaType;
			m_Callback = rhs.m_Callback;
			m_MediaModule = rhs.m_MediaModule;
			m_SessionId = rhs.m_SessionId;
		}
		return *this;
	}

	VxGUID						m_OnlineId;
	EMediaInputType				m_MediaType{ eMediaInputNone };
	MediaCallbackInterface*		m_Callback{ nullptr };
	EMediaModule				m_MediaModule{ eMediaModuleInvalid };
	VxGUID						m_SessionId;
};

//#define DEBUG_PROCESSOR_LOCK 1
//#define DEBUG_AUDIO_PROCESSOR_LOCK 1
class MediaProcessor : public MediaCallbackInterface
{
public:
	class AudioProcessorLock
	{
	public:
		AudioProcessorLock( MediaProcessor * processor ) : m_Mutex(processor->getAudioMutex())	
		{ 
#ifdef DEBUG_AUDIO_PROCESSOR_LOCK
	LogMsg( LOG_INFO, "AudioProcessorLock Lock start");
#endif //DEBUG_AUDIO_PROCESSOR_LOCK
			m_Mutex.lock(); 
#ifdef DEBUG_AUDIO_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "AudioProcessorLock Lock complete");
#endif //DEBUG_AUDIO_PROCESSOR_LOCK
		}

		~AudioProcessorLock()																		
		{ 
#ifdef DEBUG_AUDIO_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "AudioProcessorLock Unlock");
#endif //DEBUG_AUDIO_PROCESSOR_LOCK
			m_Mutex.unlock(); 
		}

		VxMutex&				m_Mutex;
	};

	class VideoProcessorLock
	{
	public:
		VideoProcessorLock( MediaProcessor * processor ) : m_Mutex(processor->getVideoMutex())	
		{ 
#ifdef DEBUG_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "VideoProcessorLock Lock start");
#endif //DEBUG_PROCESSOR_LOCK
			m_Mutex.lock(); 
#ifdef DEBUG_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "VideoProcessorLock Lock complete");
#endif //DEBUG_PROCESSOR_LOCK
		}

		~VideoProcessorLock()																		
		{ 
#ifdef DEBUG_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "VideoProcessorLock Unlock");
#endif //DEBUG_PROCESSOR_LOCK
			m_Mutex.unlock(); 
		}

		VxMutex&				m_Mutex;
	};

	class MixerProcessorLock
	{
	public:
		MixerProcessorLock( MediaProcessor* processor ) : m_Mutex( processor->getMixerMutex() )
		{
#ifdef DEBUG_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "MixerProcessorLock Lock start" );
#endif //DEBUG_PROCESSOR_LOCK
			m_Mutex.lock();
#ifdef DEBUG_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "MixerProcessorLock Lock complete" );
#endif //DEBUG_PROCESSOR_LOCK
		}

		~MixerProcessorLock()
		{
#ifdef DEBUG_PROCESSOR_LOCK
			LogMsg( LOG_INFO, "MixerProcessorLock Unlock" );
#endif //DEBUG_PROCESSOR_LOCK
			m_Mutex.unlock();
		}

		VxMutex& m_Mutex;
	};


	MediaProcessor( P2PEngine& engine );
	virtual ~MediaProcessor();
	
	void						shutdownMediaProcessor( void );

	MediaTools&					getMediaTools( void ) { return m_MediaTools; }

	VxMutex&					getMixerMutex( void )				{ return m_MixerClientsMutex; }
	VxMutex&					getAudioMutex( void )				{ return m_AudioMutex; }
	VxMutex&					getVideoMutex( void )				{ return m_VideoMutex; }

	bool						isSpeakerOutputEnabled( void )		{ return m_SpeakerOutputEnabled; }
	bool						isMicrophoneCaptureEnabled( void )	{ return m_MicCaptureEnabled; }
	bool						isVideoCaptureEnabled( void )		{ return m_VidCaptureEnabled; }

	void						wantMediaInput( VxGUID&						onlineId,
												EMediaInputType				mediaType, 
												MediaCallbackInterface *	callback, 
												EMediaModule				mediaModule,
												VxGUID&						sessionId,
												bool						wantInput );

	void						fromGuiEchoCanceledSamplesThreaded( const AudioPcmData& audioPcmData );
	void						fromGuiAudioOutSpaceAvaiThreaded( int freeSpaceLenBytes );

	void						increasePcmSampleVolume( int16_t * pcmData, uint16_t pcmDataLen, float volumePercent0To100 );
	void						playAudio( int16_t * pcmData, int dataLenInBytes );

	void						muteSpeaker( bool muteSpeaker )							{ m_MuteSpeaker = muteSpeaker; }
	bool						isSpeakerMuted( void )									{ return m_MuteSpeaker; }
	void						muteMicrophone( bool muteMic )							{ m_MuteMicrophone = muteMic; }
	bool						isMicrophoneMuted( void )								{ return m_MuteMicrophone; }

	void						processCamCaptureJpgVideo( std::shared_ptr<CamJpgVideo>& jpgVideo );
	void						sendCamPackets( std::shared_ptr<CamJpgVideo>& jpgVideo );
	void						sendJpgVideo( VxGUID& onlineId, std::shared_ptr<CamJpgVideo>& jpgVideo );

	void						processFriendVideoFeed(	VxGUID&			onlineId, 
														uint8_t *		jpgData, 
														uint32_t		jpgDataLen,
														int				motion0To100000 );

	void						processFriendAudioFeed(	VxGUID&	onlineId, int16_t * pcmData, uint16_t pcmDataLen, bool dontLock = false );

	void						processAudioInThreaded( void );

	void 						setMyIdInVidPktListCount( int cnt ) { m_VidPktListContainsMyIdCnt = cnt; }
	int							getMyIdInVidPktListCount( void )	{ return m_VidPktListContainsMyIdCnt; }

protected:
	void						processRawAudioIn( const AudioPcmData& audioPcmData );
	bool						isAudioMediaType( EMediaInputType mediaType );
	void						wantAudioMediaInput(	VxGUID&						onlineId,
														EMediaInputType				mediaType, 
														MediaCallbackInterface *	callback, 
														EMediaModule				mediaModule,
														VxGUID&						sessionId,
														bool						wantInput );

	void						wantMixerMediaInput(	VxGUID&						onlineId,
														EMediaInputType				mediaType, 
														MediaCallbackInterface *	callback, 
														EMediaModule				mediaModule,
														VxGUID&						sessionId,
														bool						wantInput );

	void						wantVideoMediaInput(	VxGUID&						onlineId,
														EMediaInputType				mediaType, 
														MediaCallbackInterface *	callback, 
														EMediaModule				mediaModule,
														VxGUID&						sessionId,
														bool						wantInput );
	
	void						doMixerClientRemovals( std::vector<ClientToRemove>& clientRemoveList );
	void						doAudioClientRemovals( std::vector<ClientToRemove>& clientRemoveList );
	void						doVideoClientRemovals( std::vector<ClientToRemove>& clientRemoveList );

	bool						clientExistsInList(	std::vector<MediaClient>&		clientList, 
													VxGUID&							onlineId,
													EMediaModule					mediaModule,
													EMediaInputType					mediaType, 
													VxGUID&							sessionId,
													MediaCallbackInterface *		callback );

	bool						removeClientFromListist( std::vector<MediaClient>&		clientList,
														 VxGUID&						onlineId,
														 EMediaModule					mediaModule,
														 EMediaInputType				mediaType,
														 VxGUID&						sessionId,
														 MediaCallbackInterface*		callback );

	bool						clientToRemoveExistsInList(	std::vector<ClientToRemove>&	clientRemoveList, 
															VxGUID&							onlineId,
															EMediaModule					mediaModule,
															EMediaInputType					mediaType, 
															VxGUID&							sessionId,
															MediaCallbackInterface *		callback );

	bool						clientToRemoveRemoveFromList(	std::vector<ClientToRemove>&	clientRemoveList, 
																VxGUID&							onlineId,
																EMediaModule					mediaModule,
																EMediaInputType					mediaType, 
																VxGUID&							sessionId,
																MediaCallbackInterface *		callback );

	std::vector<MediaClient>*	getClientList( EMediaInputType mediaInputType );

	EMediaInputCategory			getMediaInputCategory( EMediaInputType mediaInputType );

	void						lockClientList( EMediaInputCategory mediaCategory );
	void						unlockClientList( EMediaInputCategory mediaCategory );

	int							countMediaModuleRequests( EMediaInputType mediaType, EMediaModule mediaModule );
    int							countClientModuleTypeRequests( EMediaModule mediaModule, std::vector<MediaClient>* clientList );


	//=== vars ===//
	P2PEngine&					m_Engine;
	
	MediaTools&					m_MediaTools;
	VxMutex						m_AudioMutex;
	VxMutex						m_VideoMutex;

	std::vector<PluginBase*>	m_aoWantAppIdle;				// list of plugins that want called on app idle

	std::vector<AudioPcmData>	m_ProcessAudioQue;
	VxMutex						m_AudioQueInMutex;
	VxThread					m_ProcessAudioInThread;
	VxSemaphore					m_AudioInSemaphore;
	PktVoiceReq					m_PktVoiceReq;

	std::vector<MediaClient>	m_MixerList;
	VxMutex						m_MixerClientsMutex;

	std::vector<MediaClient>	m_AudioPcmList;
	std::vector<MediaClient>	m_AudioOpusList;
	std::vector<MediaClient>	m_AudioPktsList;

	std::vector<MediaClient>	m_VideoJpgList;
	std::vector<MediaClient>	m_VideoPktsList;

	std::vector<ClientToRemove>	m_MixerClientRemoveList;
	VxMutex						m_MixerRemoveMutex;
	std::vector<ClientToRemove>	m_VideoClientRemoveList;
	VxMutex						m_VideoRemoveMutex;
	std::vector<ClientToRemove>	m_AudioClientRemoveList;
	VxMutex						m_AudioRemoveMutex;

	PktVideoFeedPic *			m_PktVideoFeedPic{ nullptr };
	std::vector<PktVideoFeedPicChunk *> m_VidChunkList;

	VxMutex						m_MixerBufferMutex;

	int16_t						m_QuietAudioBuf[ AUDIO_SAMPLES_PER_FRAME ];
	int16_t						m_MixerBuf[ AUDIO_SAMPLES_PER_FRAME ];
	bool						m_MixerBufUsed{ false };

	bool						m_MuteSpeaker{ false };
	bool						m_MuteMicrophone{ false };
	bool						m_VidCaptureEnabled{ false };
	bool						m_MicCaptureEnabled{ false };
	bool						m_SpeakerOutputEnabled{ false };

	int							m_VidPktListContainsMyIdCnt{ 0 };

	MediaCallbackInterface*		m_GuiPlayerCallback = nullptr;

	OpusCodec					m_OpusCodec;
};
