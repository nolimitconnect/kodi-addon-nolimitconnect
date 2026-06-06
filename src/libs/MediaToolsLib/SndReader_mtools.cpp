//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SndReader.h"
#include <opus/OpusCodec.h>
#include "OpusFileDecoder.h"

#include <P2PEngine/P2PEngine.h>
#include <AssetMgr/AssetInfo.h>
#include <MediaProcessor/MediaProcessor.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxMacros.h>

//============================================================================
SndReader::SndReader( P2PEngine& engine, MediaProcessor& mediaProcessor )
: m_Engine( engine )
, m_MediaProcessor( mediaProcessor )
, m_FileName( "" )
, m_OpusCodec( * ( new OpusCodec( AUDIO_DEVICE_SAMPLE_RATE, 1 ) ) )
, m_OpusFileDecoder( * ( new OpusFileDecoder( engine, mediaProcessor ) ) )
{
}

//============================================================================
SndReader::~SndReader()
{
	delete &m_OpusFileDecoder;
	delete &m_OpusCodec;
}

//============================================================================
bool SndReader::fromGuiIsNoLimitAudioFile( const char* fileName )
{
	uint64_t fileLen = VxFileUtil::getFileLen( fileName );
	if( fileLen < 1000 )
	{
		return false;
	}

	VFile * fileHandle = VFileOpen( fileName, "rb" );
	if( 0 == fileHandle )
	{
		LogMsg( LOG_ERROR, "SndReader::fromGuiIsNoLimitAudioFile could not open file %s", fileName );
		return false;
	}

	const uint32_t OPUS_NOLIMIT_SIGNATURE_OFFSET = NO_LIMIT_OPUS_SIGNITURE_OFFS;

	if( 0 != VxFileUtil::fileSeek( fileHandle, OPUS_NOLIMIT_SIGNATURE_OFFSET ) )
	{
		VFileClose( fileHandle );
		LogMsg( LOG_ERROR, "SndReader::fromGuiIsNoLimitAudioFile could not seek file %s", fileName );
		return false;
	}

	// at 0x9c ( should be signature nolimitconnect.com v0000000000000000-XXv where the zeros are hex ascii of total snd frames and XX is version number

	char junkBuf[ 18 ];
	if( 18 != VFileRead( junkBuf, 1, 18, fileHandle ) )
	{
		VFileClose( fileHandle );
		LogMsg( LOG_ERROR, "SndReader::fromGuiIsNoLimitAudioFile could not read file %s", fileName );
		return false;
	}

	VFileClose( fileHandle );
	if( 0 != strncmp( junkBuf, "nolimitconnect.", 15 ) )
	{
		LogMsg( LOG_INFO, "SndReader::fromGuiIsNoLimitAudioFile is not No Limit Audio File %s", fileName );
		return false;
	}

	return true;
}

//============================================================================
bool SndReader::fromGuiAssetAction( AssetBaseInfo& assetInfo, EAssetAction assetAction, int pos0to100000 )
{
	bool result = false;
	switch( assetAction )
	{
	case eAssetActionPlayBegin:
		if( m_AssetId.isValid() )
		{
			fromGuiSndPlay( eSndPlayStateStopPlaying, m_AssetId, 0 );
		}

		m_FileName	= assetInfo.getAssetNameAndPath();
		m_AssetId	= assetInfo.getAssetUniqueId();
		return fromGuiSndPlay( eSndPlayStateStartPlaying, assetInfo.getAssetUniqueId(), pos0to100000 );

	case eAssetActionPlayEnd:
		if( m_AssetId == assetInfo.getAssetUniqueId() )
		{
			return fromGuiSndPlay( eSndPlayStateStopPlaying, assetInfo.getAssetUniqueId(), pos0to100000 );
		}

		break;
	case eAssetActionPlayCancel:
		if( m_AssetId == assetInfo.getAssetUniqueId() )
		{
			fromGuiSndPlay( eSndPlayStateStopPlaying, assetInfo.getAssetUniqueId(), pos0to100000 );
		}

		break;

	default:
		break;
	}

	return result;

}

//============================================================================
bool SndReader::fromGuiSndPlay( ESndPlayState eRecState, VxGUID& assetId, int pos0to100000 )
{
	bool result = false;
	switch( eRecState )
	{
	case eSndPlayStateStopPlaying:
		if( getIsPlaying() )
		{
			stopSndRead();
			result = true;
		}

		break;

	case eSndPlayStateStartPlaying:
		if( getIsPlaying() )
		{
			stopSndRead();
		}
			
		result = startSndRead( m_FileName.c_str(), assetId, pos0to100000 );
		break;

	case eSndPlayStateStartPlayInSeekPos:
		if( getIsPlaying() )
		{
			stopSndRead();
		}

		result = startSndRead( m_FileName.c_str(), assetId, pos0to100000 );
		break;

	case eSndPlayStatePausePlaying:
		setIsPlayingPaused( true );
		break;

	case eSndPlayStateResumePlaying:
		setIsPlayingPaused( false );
		break;

	case eSndPlayStateCancelPlaying:
		result = true;
		if( getIsPlaying() )
		{
			stopSndRead();
		}

		break;

	case eSndPlayStateDisabled:
	case eSndPlayStateError:
	default:
		break;
	}

	return result;
}

//============================================================================
bool SndReader::startSndRead( const char* fileName, VxGUID& assetId, int pos0to100000 )
{
	stopSndRead();
	bool result = m_OpusFileDecoder.beginFileDecode( fileName, assetId, pos0to100000 );
	if( result )
	{
		setIsPlayingPaused( false );
		setIsPlaying( true );
	}

	return result;
}

//============================================================================
void SndReader::setIsPlayingPaused( bool pause )
{
	if( pause )
	{
		if( getIsPlaying() )
		{
			if( !m_IsFirstFrameAfterResumePlaying )
			{
				m_TotalElapsedMs += m_PlayElapseTimer.elapsedMs();
			}
		}
		
		m_IsPlayingPaused = true;
	}
	else
	{
		m_IsFirstFrameAfterResumePlaying = true;
		m_IsPlayingPaused = false;
	}
}

//============================================================================
void SndReader::stopSndRead( void )
{
	if( getIsPlaying() )
	{
		setIsPlayingPaused( true );
		m_OpusFileDecoder.finishFileDecode( true );
		setIsPlaying( false );
	}
}

//============================================================================
void SndReader::closeSndFile( void )
{
	if( 0 != m_FileHandle )
	{
		VFileClose( m_FileHandle );
		m_FileHandle = 0;
	}
}
