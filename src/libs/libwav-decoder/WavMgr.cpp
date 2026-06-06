//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "WavMgr.h"

#include "wav_decoder.h"

#include <CoreLib/VxDebug.h>
#include <CoreLib/VFile.h>

#include <algorithm>

namespace
{
    const char* DescribeWavResult( enum wav_decoder::WAVDecoderResult result )
	{
		switch( result )
		{
		case wav_decoder::WAV_DECODER_SUCCESS_NEXT:
			return "WAV_DECODER_SUCCESS_NEXT";
		case wav_decoder::WAV_DECODER_SUCCESS_IN_DATA:
			return "WAV_DECODER_SUCCESS_IN_DATA";
		case wav_decoder::WAV_DECODER_ERROR_NO_RIFF:
			return "WAV_DECODER_ERROR_NO_RIFF";
		case wav_decoder::WAV_DECODER_ERROR_NO_WAVE:
			return "WAV_DECODER_ERROR_NO_WAVE";
		default:
			return "Unknown WAVDecoderResult";
		}
	}
};

bool WavMgr::readWavFile( std::string& fileName, std::vector<int16_t>& retBytes, int& retRate, int& retChannels, int& retBitsPerSample )
{
	retBytes.clear();
	VFile* vFile = VFileOpen( fileName.c_str(), "rb" );
	if( !vFile )
	{
		LogMsg( LOG_ERROR, "%s Could not open file %s", __func__, fileName.c_str() );
		return false;
	}

	const std::size_t buffer_size = wav_decoder::min_buffer_size;
	uint8_t buffer[buffer_size];
	wav_decoder::WAVDecoder decoder( buffer );

	std::size_t bytes_to_read = decoder.bytes_needed();
	std::size_t bytes_to_skip = decoder.bytes_to_skip();
	std::size_t buffer_offset = 0;
	std::size_t bytes_read = 0;

	wav_decoder::WAVDecoderResult result = wav_decoder::WAV_DECODER_SUCCESS_NEXT;
	while( result == wav_decoder::WAV_DECODER_SUCCESS_NEXT ) {
		// Skip unneeded data
		if( bytes_to_skip > 0 ) {
			bytes_read = VFileRead( buffer, 1, std::min( bytes_to_skip, buffer_size ), vFile );

			if( bytes_read == 0 ) {
				LogMsg( LOG_ERROR, "%s Out of data", __func__ );
				VFileClose( vFile );
				return false;
			}

			bytes_to_skip -= bytes_read;
			continue;
		}

		// Read needed data
		bytes_read = VFileRead( buffer + buffer_offset, 1, std::min( bytes_to_read, buffer_size - buffer_offset ), vFile );

		if( bytes_read == 0 ) {
			LogMsg( LOG_ERROR, "%s Out of data2", __func__ );
			VFileClose( vFile );
			return false;
		}

		bytes_to_read -= bytes_read;
		buffer_offset += bytes_read;

		if( bytes_to_read > 0 ) {
			continue;
		}

		result = decoder.next();
		if( result == wav_decoder::WAV_DECODER_SUCCESS_IN_DATA ) {
			break;
		}

		bytes_to_skip = decoder.bytes_to_skip();
		bytes_to_read = decoder.bytes_needed();
		buffer_offset = 0;
	}

	if( result == wav_decoder::WAV_DECODER_SUCCESS_IN_DATA ) {
		retRate = decoder.sample_rate();
		retChannels = decoder.num_channels();
		retBitsPerSample = decoder.bits_per_sample();
		// LogMsg( LOG_VERBOSE, "%s Sample rate: %d", __func__, retRate );
		// LogMsg( LOG_VERBOSE, "%s Channels: %d", __func__, retChannels );
		// LogMsg( LOG_VERBOSE, "%s Bits per samples: %d", __func__, retBitsPerSample );

		std::size_t num_samples =
			decoder.chunk_bytes_left() /
			(decoder.num_channels() * (decoder.bits_per_sample() / 8));
		// LogMsg( LOG_VERBOSE, "%s Samples: %d", __func__, num_samples );

		if( retChannels != 1 || retBitsPerSample != 16 )
		{
			LogMsg( LOG_VERBOSE, "%s ERROR only mono channed 16 bit pcm wave files can be read", __func__);
			VFileClose( vFile );
			return false;
		}

		retBytes.resize( num_samples );
		int bytes_read = VFileRead( retBytes.data(), 1, num_samples * 2, vFile );
		if( bytes_read != num_samples * 2 )
		{
			LogMsg( LOG_VERBOSE, "%s ERROR expected %d bytes got %d bytes", __func__, num_samples * 2, bytes_read );
			VFileClose( vFile );
			return false;
		}
	}
	else {
		LogMsg( LOG_VERBOSE, "%s Unexpected error: %s", __func__, DescribeWavResult( result ) );
		VFileClose( vFile );
		return false;
	}

	VFileClose( vFile );
	return true;
}
