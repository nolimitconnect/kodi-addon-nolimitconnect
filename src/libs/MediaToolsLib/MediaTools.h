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

#include <CoreLib/MediaCallbackInterface.h>
#include <GuiInterface/IFromGui.h>

#include <string>

class P2PEngine;
class MediaProcessor;
class IToGui;
class OpusCodec;
class SndWriter;
class SndReader;
class MJPEGWriter;
class MJPEGReader;

class MediaTools : public MediaCallbackInterface
{
public:
	MediaTools( P2PEngine& engine, MediaProcessor& mediaProcessor );
	virtual ~MediaTools();

	OpusCodec&					getOpusCodec( void )					{ return m_OpusCodec; }
	SndWriter&					getAudioWriter( void )					{ return m_AudioWriter; }
	SndReader&					getAudioReader( void )					{ return m_AudioReader; }
	MJPEGWriter&				getVideoWriter( void )					{ return m_VideoWriter; }
	MJPEGReader&				getVideoReader( void )					{ return m_VideoReader; }

	bool						fromGuiIsNoLimitVideoFile( const char* fileName );	
	bool						fromGuiIsNoLimitAudioFile( const char* fileName );	
	bool						fromGuiSndRecord( ESndRecordState eRecState, VxGUID& feedId, const char* fileName );
	bool						fromGuiVideoRecord( EVideoRecordState eRecState, VxGUID& feedId, const char* fileName );
	bool						fromGuiAssetAction( AssetBaseInfo& assetInfo, EAssetAction assetAction, int pos0to100000 );

protected:

	//=== vars ===//
	P2PEngine&					m_Engine;
	MediaProcessor&				m_MediaProcessor;
	IToGui&						getToGui();
	OpusCodec&					m_OpusCodec;
	SndWriter&					m_AudioWriter;
	SndReader&					m_AudioReader;
	MJPEGWriter&				m_VideoWriter;
	MJPEGReader&				m_VideoReader;
};
