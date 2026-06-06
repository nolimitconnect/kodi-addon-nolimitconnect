#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <memory>
#include <atomic>
#include <cstdint>

#include <GuiInterface/IDefs.h>

class CamJpgVideo
{
public:
    CamJpgVideo( std::shared_ptr<uint8_t>& vidData,
                int	vidDataLen,
                int motion = 0,
                uint64_t frameTag = 0,
                EMediaModule sourceModule = eMediaModuleInvalid )
        : m_VidData( vidData )
		, m_VidDataLen( vidDataLen )
        , m_Motion( motion )
		, m_FrameTag( frameTag ? frameTag : getNextFrameTag() )
		, m_SourceModule( sourceModule )
	{
	}

	std::shared_ptr<uint8_t>	m_VidData;
    int                         m_VidDataLen;
    int							m_Motion;
    uint64_t                    m_FrameTag;
    EMediaModule                m_SourceModule;

private:
    static uint64_t getNextFrameTag( void )
    {
        static std::atomic<uint64_t> s_NextFrameTag{ 1 };
        return s_NextFrameTag.fetch_add( 1, std::memory_order_relaxed );
    }
};
