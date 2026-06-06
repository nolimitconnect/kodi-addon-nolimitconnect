//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "MediaTools.h"

#include "SndWriter.h"
#include "SndReader.h"
#include "MJPEGWriter.h"
#include "MJPEGReader.h"
#include <GuiInterface/IToGui.h>
#include <GuiInterface/IAudioDefs.h>
#include <opus/OpusCodec.h>

#include <AssetMgr/AssetInfo.h>

//============================================================================
MediaTools::MediaTools( P2PEngine& engine, MediaProcessor& mediaProcessor )
: m_Engine( engine )
, m_MediaProcessor( mediaProcessor )
, m_OpusCodec(  * ( new OpusCodec( AUDIO_DEVICE_SAMPLE_RATE, AUDIO_CHANNELS ) ) )
, m_AudioWriter(  * ( new SndWriter( engine, mediaProcessor ) ) )
, m_AudioReader(  * ( new SndReader( engine, mediaProcessor ) ) )
, m_VideoWriter(  * ( new MJPEGWriter( engine, mediaProcessor ) ) )
, m_VideoReader(  * ( new MJPEGReader( engine, mediaProcessor ) ) )
{
}

//============================================================================
MediaTools::~MediaTools()
{
	delete &m_OpusCodec;
	delete &m_AudioWriter;
	delete &m_VideoWriter;
}

//============================================================================
IToGui&	MediaTools::getToGui()
{
    return IToGui::getIToGui();
}

//============================================================================
bool MediaTools::fromGuiIsNoLimitVideoFile( const char* fileName )
{
	return m_VideoReader.fromGuiIsNoLimitVideoFile( fileName );
}

//============================================================================
bool MediaTools::fromGuiIsNoLimitAudioFile( const char* fileName )
{
	return m_AudioReader.fromGuiIsNoLimitAudioFile( fileName );
}

//============================================================================
bool MediaTools::fromGuiSndRecord( enum ESndRecordState eRecState, VxGUID& feedId, const char* fileName )
{
	return m_AudioWriter.fromGuiSndRecord( eRecState, feedId, fileName );
}

//============================================================================
bool MediaTools::fromGuiVideoRecord( enum EVideoRecordState eRecState, VxGUID& feedId, const char* fileName  )
{
	return m_VideoWriter.fromGuiVideoRecord( eRecState, feedId, fileName );
}

//============================================================================
bool MediaTools::fromGuiAssetAction( AssetBaseInfo& assetInfo, enum EAssetAction assetAction, int pos0to100000 )
{
	bool result = false;
	switch( assetAction )
	{
	case eAssetActionRecordBegin:
	case eAssetActionRecordPause:
	case eAssetActionRecordResume:
	case eAssetActionRecordEnd:
	case eAssetActionRecordCancel:
		if( eAssetTypeAudio == assetInfo.getAssetType() )
		{
			result = m_AudioWriter.fromGuiAssetAction( assetInfo,  assetAction, pos0to100000 );
		}
		break;
	case eAssetActionPlayBegin:
	case eAssetActionPlayOneFrame:
	case eAssetActionPlayPause:
	case eAssetActionPlayResume:
	case eAssetActionPlayProgress:
	case eAssetActionPlayEnd:
	case eAssetActionPlayCancel:
		if( eAssetTypeAudio == assetInfo.getAssetType() )
		{
			result = m_AudioReader.fromGuiAssetAction( assetInfo, assetAction, pos0to100000 );
		}
		else if( eAssetTypeVideo == assetInfo.getAssetType() )
		{
			result = m_VideoReader.fromGuiAssetAction( assetInfo, assetAction, pos0to100000 );
		}
		break;
	default:
		break;
	}

	return result;
}
