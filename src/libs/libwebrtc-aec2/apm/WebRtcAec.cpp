//============================================================================
// Copyright (C) 2026 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "WebRtcAec.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <vector>

#include "webrtc/modules/audio_processing/aec/include/echo_cancellation.h"

class WebRtcAec::Impl {
public:
    void* aec{nullptr};
    int sampleRateHz{16000};
    size_t channels{1};
    size_t frameSize{160};
    int currentEchoDelayMs{100};

    float lastVadProbability{0.0f};
    float lastEchoReturnLoss{0.0f};

    std::vector<float> nearFloat;
    std::vector<float> farFloat;
    std::vector<float> outFloat;
    mutable std::mutex mutex;

    bool initialized{false};
};

namespace {

bool IsSupportedLegacyAecRate(int sampleRateHz)
{
    return sampleRateHz == 8000 || sampleRateHz == 16000;
}

void ResetAecInstance(void*& aec,
                      int sampleRateHz,
                      size_t channels,
                      bool& initialized)
{
    if (aec) {
        WebRtcAec_Free(aec);
        aec = nullptr;
    }
    initialized = false;

    if (!IsSupportedLegacyAecRate(sampleRateHz) || channels != 1) {
        return;
    }

    if (WebRtcAec_Create(&aec) != 0 || aec == nullptr) {
        aec = nullptr;
        return;
    }

    if (WebRtcAec_Init(aec, sampleRateHz, sampleRateHz) != 0) {
        WebRtcAec_Free(aec);
        aec = nullptr;
        return;
    }

    AecConfig cfg;
    cfg.nlpMode = kAecNlpModerate;
    cfg.skewMode = kAecFalse;
    cfg.metricsMode = kAecTrue;
    cfg.delay_logging = kAecFalse;
    WebRtcAec_set_config(aec, cfg);

    initialized = true;
}

void PrepareScratch(std::vector<float>& buffer, int16_t* data, int frames)
{
    buffer.resize(static_cast<size_t>(frames));
    for (int i = 0; i < frames; ++i) {
        buffer[static_cast<size_t>(i)] = static_cast<float>(data[i]);
    }
}

void WriteScratchToInt16(const std::vector<float>& buffer, int16_t* data, int frames)
{
    for (int i = 0; i < frames; ++i) {
        const float sample = std::clamp(buffer[static_cast<size_t>(i)], -32768.0f, 32767.0f);
        data[i] = static_cast<int16_t>(std::lround(sample));
    }
}

} // namespace

WebRtcAec::WebRtcAec()
    : m_impl(std::make_unique<Impl>())
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    ResetAecInstance(m_impl->aec, m_impl->sampleRateHz, m_impl->channels, m_impl->initialized);
}

WebRtcAec::~WebRtcAec()
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (m_impl->aec) {
        WebRtcAec_Free(m_impl->aec);
        m_impl->aec = nullptr;
    }
}

void WebRtcAec::setStreamFormat(int sample_rate_hz, size_t channels)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (sample_rate_hz <= 0 || channels == 0) {
        return;
    }

    if (m_impl->sampleRateHz == sample_rate_hz && m_impl->channels == channels) {
        return;
    }

    m_impl->sampleRateHz = sample_rate_hz;
    m_impl->channels = channels;
    m_impl->frameSize = static_cast<size_t>(sample_rate_hz / 100);

    ResetAecInstance(m_impl->aec, m_impl->sampleRateHz, m_impl->channels, m_impl->initialized);
}

void WebRtcAec::setAgcEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_agcEnabled = enabled;
}

bool WebRtcAec::isAgcEnabled(void) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_agcEnabled;
}

void WebRtcAec::setEchoDelay(int delayMs)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->currentEchoDelayMs = delayMs;
}

float WebRtcAec::lastVadProbability(void) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->lastVadProbability;
}

float WebRtcAec::lastEchoReturnLoss(void) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->lastEchoReturnLoss;
}

void WebRtcAec::processRender(int16_t* data, int frames)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->initialized || !m_impl->aec || !data || frames <= 0) {
        return;
    }

    if (m_impl->channels != 1 || static_cast<size_t>(frames) != m_impl->frameSize) {
        return;
    }

    if (frames != 80 && frames != 160) {
        return;
    }

    PrepareScratch(m_impl->farFloat, data, frames);
    WebRtcAec_BufferFarend(m_impl->aec, m_impl->farFloat.data(), static_cast<int16_t>(frames));
}

void WebRtcAec::processCapture(int16_t* data, int frames)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->initialized || !m_impl->aec || !data || frames <= 0) {
        return;
    }

    if (m_impl->channels != 1 || static_cast<size_t>(frames) != m_impl->frameSize) {
        return;
    }

    if (frames != 80 && frames != 160) {
        return;
    }

    PrepareScratch(m_impl->nearFloat, data, frames);
    m_impl->outFloat.resize(static_cast<size_t>(frames));

    const float* nearBand[1] = {m_impl->nearFloat.data()};
    float* outBand[1] = {m_impl->outFloat.data()};

    if (WebRtcAec_Process(m_impl->aec,
                          nearBand,
                          1,
                          outBand,
                          static_cast<int16_t>(frames),
                          static_cast<int16_t>(m_impl->currentEchoDelayMs),
                          0) != 0) {
        return;
    }

    WriteScratchToInt16(m_impl->outFloat, data, frames);

    AecMetrics metrics;
    if (WebRtcAec_GetMetrics(m_impl->aec, &metrics) == 0) {
        m_impl->lastEchoReturnLoss = static_cast<float>(metrics.erl.average);
    }

    int echoStatus = 0;
    if (WebRtcAec_get_echo_status(m_impl->aec, &echoStatus) == 0) {
        m_impl->lastVadProbability = echoStatus != 0 ? 1.0f : 0.0f;
    }
}
