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

#include <config_appcorelibs.h>

#include <MediaProcessor/MediaProcessor.h>
#include "OpusCallbackInterface.h"
#include "SndDefs.h"
#include "MyOpusHeader.h"

#include <opus/include/opus.h>
#include <opus/include/opus_multistream.h>
#include <libogg/include/ogg/ogg.h>

#include "OpusTools/speex_resampler.h"

#include <string>

typedef struct shapestate shapestate;
struct shapestate 
{
	float * b_buf;
	float * a_buf;
	int fs;
	int mute;
};

class OpusCodec;
class OggStream;
class P2PEngine;
class MediaProcessor;
class VFile;

class OpusFileDecoder : public MediaCallbackInterface
{
public:
	OpusFileDecoder( P2PEngine& engine, MediaProcessor& mediaProcessor );
	virtual ~OpusFileDecoder();
 
	bool						beginFileDecode( const char* fileName, VxGUID& assetId, int pos0to100000 );
	int							decodedNextFrame( uint8_t * frameBuffer, int frameBufferLen );
	void						finishFileDecode( bool abortedByUser = false );

	virtual void				callbackAudioOutSpaceAvail( int freeSpaceLenBytes );

protected:
	int							moveOpusFramesToOutput( uint8_t * outBuffer );
	bool						processOggFileHeader( MyOpusHeader& header, ogg_packet *op, float manualGain );

	int							opusPcmOutputToPcm( int16_t*		opusOutput,
													int				channels,
													int				frame_size,
													SpeexResamplerState* resampler,
													int*			skip,
													opus_int64		maxout );

	bool						seekOpusFile( VFile * fileHandle, int pos0to100000 );
	bool						readTotalSndFrames( VFile * fileHandle );
	bool						seekFile( VFile * fileHandle, uint64_t filePosition );

	int							calculateFileProgress( void );
	void						clearDecodedFrames( void );
	void						enableSpaceAvailCallback( bool enableCallback, bool lockResources );

	//=== vars ===//
	P2PEngine&					m_Engine;
	MediaProcessor&				m_MediaProcessor;
	OpusDecoder *				m_OpusCodec;
	std::string					m_FileName;
	VxGUID						m_AssetId; 
	uint64_t					m_FileLen{ 0 };
	uint64_t					m_FilePos{ 0 };
	VFile *						m_FileHandle{ nullptr };
	uint64_t					m_TotalSndFramesInFile{ 0 };
	uint64_t					m_ConsumedSndFrames{ 0 };
	bool						m_DecoderInitialized{ false };
	bool						m_InputInitialized{ false };

	MyOpusHeader				m_MyOpusHeader;
	ogg_sync_state				m_OggSyncState;		// oy;
	ogg_stream_state			m_StreamState;		// os
	ogg_packet					m_OggPkt;			// op;
	ogg_page					m_OggPage;			// og
	bool						m_StreamInit{ false };
	int							m_HasOpusStream{ 0 };
	int							m_HasTagsPacket{ 0 };
	ogg_int32_t					m_OpusSerialNum{ 0 };
	int							m_Eos{ 0 };
	int							m_PacketCount{ 0 };
	opus_int64					m_LinkOut{ 0 };
	float						m_ManualGain{ 0 };
	int							m_Channels{ AUDIO_CHANNELS };
	int							m_Rate{ AUDIO_DEVICE_SAMPLE_RATE };
	int							m_Preskip{ 0 };
	int 						m_GranOffset{ 0 };
	SpeexResamplerState *		m_Resampler{ nullptr };
	int16_t*					m_OpusOutput{ nullptr };
	int							m_DecodedSampleCnt;
	std::vector<char *>			m_DecodedFrames;
	bool						m_FirstDecodedFrame{ false };
	bool						m_HeaderHasBeenRead{ false };
	VxMutex						m_ResourceMutex;
	bool						m_SpaceAvailCallbackEnabled{ false };
	VxGUID						m_MediaSessionId;
};

