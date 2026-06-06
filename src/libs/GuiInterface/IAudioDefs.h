#pragma once
//============================================================================
// Copyright (C) 2023 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

constexpr int AUDIO_DEVICE_SAMPLE_RATE = 16000;
constexpr int AUDIO_CHANNELS = 1;	
constexpr int AUDIO_BYTES_PER_SAMPLE = 2;			// PCM 2 bytes per sample

// internal to application a frame is 60 ms
constexpr int AUDIO_MS_PER_FRAME = 60;               // 60 ms = 0.06 sec of audio data (required to be a multiple of 10ms per frame to play nice with opus and echo canceler)
constexpr int AUDIO_SAMPLES_PER_FRAME = (AUDIO_DEVICE_SAMPLE_RATE * AUDIO_CHANNELS * AUDIO_MS_PER_FRAME) / 1000; // 960 samples at 16000 Hz, 1,440 samples at 24000 Hz
constexpr int AUDIO_BUF_SIZE = AUDIO_SAMPLES_PER_FRAME * AUDIO_BYTES_PER_SAMPLE; // 1,920 bytes at 16000 Hz, 2,880 butes at 24000 Hz

constexpr double AUDIO_BYTES_TO_MS_MULTIPLIER = ((double)AUDIO_MS_PER_FRAME / (double)AUDIO_BUF_SIZE);

// echo canceler can only handle 10 ms at a time
constexpr int ECHO_MS_PER_FRAME = 10; 
constexpr int ECHO_SAMPLE_RATE = AUDIO_DEVICE_SAMPLE_RATE; 	 // echo canceler sample rate must match audio device sample rate
constexpr int ECHO_FRAME_SIZE_10MS = ECHO_SAMPLE_RATE / 100; // PCM Sample Count for 10ms at 16000 Hz is 160 samples, at 24000 Hz is 240 samples
constexpr int ECHO_SAMPLES_PER_FRAME = AUDIO_SAMPLES_PER_FRAME / ( AUDIO_MS_PER_FRAME / ECHO_MS_PER_FRAME );
constexpr int ECHO_BUF_SIZE = AUDIO_BUF_SIZE / (AUDIO_MS_PER_FRAME / ECHO_MS_PER_FRAME);


// Changes based on sample rate to maintain a constant duration in milliseconds
constexpr int OPUS_FRAME_RATE = (AUDIO_DEVICE_SAMPLE_RATE * AUDIO_MS_PER_FRAME) / 1000; 

// Opus Compressed payload size comes from bitrate/time math, not sample rate:
// constexpr int OPUS_HI_FIXED_BITRATE_BPS = 32000; // Opus wide bandwidth mode
constexpr int OPUS_LO_FIXED_BITRATE_BPS = 16000; // some distortion but I cannot tell the difference in audio quality between 16k and 32k

// 32,000 bits/sec × 0.060 seconds =  1,920 bits.
// 1,920 bits ÷ 8 bits per byte = 240 bytes.
// 6 seconds of audio / 60ms per frame = 100 frames * 240 bytes/frame = 24,000 bytes
// constexpr int OPUS_HI_COMPRESSED_BYTES_PER_FRAME = (OPUS_HI_FIXED_BITRATE_BPS * AUDIO_MS_PER_FRAME) / 8000;
// constexpr int OPUS_HI_COMPRESSED_SAMPLES_PER_FRAME = OPUS_HI_COMPRESSED_BYTES_PER_FRAME / AUDIO_BYTES_PER_SAMPLE;

// 16,000 bits/sec × 0.060 seconds = 960 bits.
// 960 bits ÷ 8 bits per byte = 120 bytes.
// 6 seconds of audio / 60ms per frame = 100 frames * 120 bytes/frame = 12,000 bytes
constexpr int OPUS_COMPRESSED_BYTES_PER_FRAME = (OPUS_LO_FIXED_BITRATE_BPS * AUDIO_MS_PER_FRAME) / 8000;
constexpr int OPUS_COMPRESSED_SAMPLES_PER_FRAME = OPUS_COMPRESSED_BYTES_PER_FRAME / AUDIO_BYTES_PER_SAMPLE;

constexpr int MIN_OPUS_FILE_LEN = 1104;


// player-nlc output (float) 960 frames 
constexpr int AUDIO_SAMPLE_RATE_KODI = 48000;        // kodi is configured for 48000 hz
constexpr int AUDIO_CHANNELS_KODI = 2;               // kodi is configured for stereo
constexpr int AUDIO_MS_KODI = 20;                    // 20 ms = 0.02 sec of audio data
constexpr float AUDIO_SEC_KODI = 0.02;               // 20 ms = 0.02 sec of audio data
constexpr int AUDIO_BYTES_PER_SAMPLE_KODI = 4;       // 4 bytes per sample (sizeof float)
// size of kodi frame in float input stream
constexpr int AUDIO_FRAME_SIZE_KODI = (int)((AUDIO_SAMPLE_RATE_KODI * ((float)AUDIO_MS_KODI/1000)) * AUDIO_BYTES_PER_SAMPLE_KODI * AUDIO_CHANNELS_KODI);   

constexpr int AUDIO_KODI_TO_NLC_DNSAMPLE_RATIO = (AUDIO_SAMPLE_RATE_KODI / AUDIO_DEVICE_SAMPLE_RATE) * (AUDIO_CHANNELS_KODI / AUDIO_CHANNELS);

