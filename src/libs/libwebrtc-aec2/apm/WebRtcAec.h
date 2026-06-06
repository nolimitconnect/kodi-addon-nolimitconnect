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

class IAudioProcessor {
public:
    virtual ~IAudioProcessor() = default;
    virtual void                processCapture(int16_t* data, int frames) = 0;
    virtual void                processRender(int16_t* data, int frames) = 0;
};

class WebRtcAec : public IAudioProcessor
{
public:
    WebRtcAec();
    ~WebRtcAec();

    void                        setStreamFormat(int sample_rate_hz, size_t channels);

    void                        setAgcEnabled(bool enabled);
    bool                        isAgcEnabled(void) const;

    void                        setEchoDelay(int delayMs);

    void                        processCapture(int16_t* data, int frames) override;
    void                        processRender(int16_t* data, int frames) override;

    float                       lastVadProbability(void) const;
    float                       lastEchoReturnLoss(void) const;

private:
    class Impl;
    std::unique_ptr<Impl>       m_impl;
    bool                        m_agcEnabled{false};
};
