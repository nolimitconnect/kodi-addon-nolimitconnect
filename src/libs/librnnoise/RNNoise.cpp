//============================================================================
// Copyright (C) 2026 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "RNNoise.h"

#include "include/rnnoise.h"
#include <GuiInterface/IAudioDefs.h>
#include <CoreLib/VxDebug.h>

#include <algorithm>


//============================================================================
RNNoise::RNNoise()
{
    // Passing NULL uses the default office-optimized model
    m_DenoiseState = rnnoise_create(NULL);
}

//============================================================================
RNNoise::~RNNoise()
{
    if (m_DenoiseState) 
    {
        rnnoise_destroy(m_DenoiseState);
        m_DenoiseState = nullptr;
    }
}

//============================================================================
void RNNoise::reduceNoise( int16_t* data, int frames )
{
    // frames must be 160 for a 16kHz source to equal the 480 required by RNNoise
    if (!m_DenoiseState || !data || frames != 160)
    {
        LogMsg( LOG_ERROR, "RNNoise::reduceNoise - Invalid parameters" );
        vx_assert(false);
        return;
    } 

    float inFloat[480];  // Upsampled buffer
    float outFloat[480]; // Denoised buffer

    // 1. UPSAMPLE: 16kHz -> 48kHz (Ratio 3)
    // We use linear interpolation to fill the "gaps" between samples
    for (int i = 0; i < 160; i++) {
        float current = static_cast<float>(data[i]);
        float next = (i < 159) ? static_cast<float>(data[i + 1]) : current;

        inFloat[i * 3 + 0] = current;
        inFloat[i * 3 + 1] = current + (next - current) * 0.333f;
        inFloat[i * 3 + 2] = current + (next - current) * 0.666f;
    }

    // 2. PROCESS: Standard RNNoise call
    m_LastSpeechProbability = rnnoise_process_frame(m_DenoiseState, outFloat, inFloat);

    // 3. DOWNSAMPLE: 48kHz -> 16kHz
    // Simple decimation (taking every 3rd sample) is usually sufficient after denoising
    for (int i = 0; i < 160; i++) {
        data[i] = static_cast<int16_t>(std::clamp(outFloat[i * 3], -32768.0f, 32767.0f));
    }

}
