//============================================================================
// Copyright (C) 2026 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OpusCodec.h"
#include <GuiInterface/IAudioDefs.h>

#include <CoreLib/VxDebug.h>
#include <stdexcept>

//============================================================================
OpusCodec::OpusCodec(int sampleRate, int channels)
{
    int error;

    encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK)
    {
        LogMsg( LOG_ERROR, "Opus encoder creation failed: %d", error );
        throw std::runtime_error("Failed to create Opus encoder");
    }

    error = opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
    if (error != OPUS_OK)
    {
        LogMsg( LOG_ERROR, "Failed to disable Opus VBR: %d", error );
        throw std::runtime_error("Failed to disable Opus VBR");
    }

    error = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_LO_FIXED_BITRATE_BPS));
    if (error != OPUS_OK)
    {
        LogMsg( LOG_ERROR, "Failed to set Opus low bandwidth bitrate: %d", error );
        throw std::runtime_error("Failed to set Opus low bandwidth bitrate");
    }

    decoder = opus_decoder_create(sampleRate, channels, &error);
    if (error != OPUS_OK)
    {
        LogMsg( LOG_ERROR, "Opus decoder creation failed: %d", error );
        throw std::runtime_error("Failed to create Opus decoder");
    }
}

//============================================================================
OpusCodec::~OpusCodec()
{
    opus_encoder_destroy(encoder);
    opus_decoder_destroy(decoder);
}

//============================================================================
int OpusCodec::encode( const int16_t* pcm, int sampleCnt, unsigned char* compressedDataBuf, int maxCompressedDataBufSize )
{
    vx_assert( pcm != nullptr );
    vx_assert( sampleCnt == AUDIO_SAMPLES_PER_FRAME );
    vx_assert( compressedDataBuf != nullptr );

    int len = opus_encode(encoder, pcm, OPUS_FRAME_RATE, compressedDataBuf, maxCompressedDataBufSize);
    if (len < 0)
    {
        LogMsg( LOG_ERROR, "Opus encode failed: %d", len );
        return 0;
    }
        
    return len;
}

//============================================================================
 // returns uncompressed data size in pcm samples or 0 if error
int OpusCodec::decode( const uint8_t* data, int dataLen, int16_t* pcmRetSamples, int maxPcmRetSamples )
{
    int samplesDecoded  = opus_decode( decoder, data, dataLen, pcmRetSamples, maxPcmRetSamples, 0 );

    if (samplesDecoded  < 0)
    {
        LogMsg( LOG_ERROR, "Opus decode failed: %d", samplesDecoded );
        return 0;
    }


    return samplesDecoded;
}
