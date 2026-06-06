#pragma once
//============================================================================
// Copyright (C) 2026 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <opus/include/opus.h>
#include <vector>
#include <stdint.h>

class OpusCodec
{
public:
    OpusCodec() = delete;
    OpusCodec(int sampleRate, int channels);
    ~OpusCodec();
    // assumes 60ms frames

    // returns compressed data size in bytes or 0 if error
    int encode( const int16_t* pcm, int sampleCnt, unsigned char* compressedDataBuf, int maxCompressedDataBufSize );

    // returns uncompressed data size in pcm samples or 0 if error
    int decode( const uint8_t* data, int dataLen, int16_t* pcmRetSamples, int maxPcmRetSamples );

private:
    OpusEncoder* encoder;
    OpusDecoder* decoder;
};
