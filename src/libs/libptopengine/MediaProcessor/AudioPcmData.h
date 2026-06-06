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

#include <GuiInterface/IDefs.h>

#include <atomic>
#include <cstdint>
#include <vector>

class AudioPcmData
{
public:
    AudioPcmData() = default;

    AudioPcmData( const int16_t* pcmData, int sampleCnt, EMediaModule sourceModule, uint64_t frameTag = 0 )
        : m_PcmData( pcmData, pcmData + sampleCnt )
        , m_SampleCnt( sampleCnt )
        , m_SourceModule( sourceModule )
        , m_FrameTag( frameTag ? frameTag : getNextFrameTag() )
    {
    }

    const int16_t* getPcmData( void ) const { return m_PcmData.data(); }
    int getSampleCnt( void ) const { return m_SampleCnt; }
    EMediaModule getSourceModule( void ) const { return m_SourceModule; }
    uint64_t getFrameTag( void ) const { return m_FrameTag; }

private:
    static uint64_t getNextFrameTag( void )
    {
        static std::atomic<uint64_t> s_NextFrameTag{ 1 };
        return s_NextFrameTag.fetch_add( 1, std::memory_order_relaxed );
    }

    std::vector<int16_t> m_PcmData;
    int m_SampleCnt{ 0 };
    EMediaModule m_SourceModule{ eMediaModuleInvalid };
    uint64_t m_FrameTag{ 0 };
};
