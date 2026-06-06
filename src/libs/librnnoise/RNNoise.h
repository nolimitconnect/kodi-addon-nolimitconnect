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

#include <cstddef>
#include <cstdint>
#include <memory>

extern "C" {
    // Forward declarations for C structs from rnnoise.h
    struct DenoiseState; 
    struct RNNModel;
}

class INoiseProcessor {
public:
    virtual ~INoiseProcessor() = default;
    
    virtual void                reduceNoise(int16_t* data, int frames) = 0;
};

class RNNoise : public INoiseProcessor
{
public:
    RNNoise();
    ~RNNoise();

    void                        reduceNoise( int16_t* data, int frames ) override;

    float                       lastSpeechProbability( void ) const{ return m_LastSpeechProbability; };

private:
    DenoiseState *              m_DenoiseState{nullptr};
    float                       m_LastSpeechProbability{0.0f};
};
