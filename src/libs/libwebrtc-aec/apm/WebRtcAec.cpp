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

#include <cstring>
#include <mutex>
#include <vector>

#include "api/audio/audio_processing.h"
#include "api/audio/builtin_audio_processing_builder.h"
#include "api/environment/environment_factory.h"
#include "api/scoped_refptr.h"

class WebRtcAec::Impl {
public:
    std::unique_ptr<webrtc::Environment> env;
    webrtc::scoped_refptr<webrtc::AudioProcessing> apm;
    int sampleRateHz{0};
    size_t channels{0};
    size_t frameSize{0};
    int currentEchoDelayMs{100};
    int lastEchoDelayMs{100};

    float lastVadProbability{0.0f};
    float lastEchoReturnLoss{0.0f};
    std::vector<int16_t> renderScratch;
    mutable std::mutex mutex;

    webrtc::StreamConfig streamConfig{0, 0};
    webrtc::ProcessingConfig processingConfig;
    webrtc::AudioProcessing::Config apConfig;
};

//============================================================================
WebRtcAec::WebRtcAec()
    : m_impl(std::make_unique<Impl>())
{
    m_impl->env = std::make_unique<webrtc::Environment>(webrtc::CreateEnvironment());

    // sound fluctuates
    // m_impl->apConfig.echo_canceller.enabled = true;
    // m_impl->apConfig.echo_canceller.mobile_mode = false;

    // sound does not fluctuate 
    // m_impl->apConfig.echo_canceller.enabled = false;
    // m_impl->apConfig.echo_canceller.mobile_mode = true;

    // sound does not fluctuate
    m_impl->apConfig.echo_canceller.enabled = true;
    m_impl->apConfig.echo_canceller.mobile_mode = true;

    // sound does not fluctuate
    // m_impl->apConfig.echo_canceller.enabled = false;
    // m_impl->apConfig.echo_canceller.mobile_mode = false;

    m_impl->apConfig.echo_canceller.enforce_high_pass_filtering = true; 
    m_impl->apConfig.high_pass_filter.enabled = true;
    m_impl->apConfig.transient_suppression.enabled = true;
    m_impl->apConfig.noise_suppression.enabled = true;
    m_impl->apConfig.noise_suppression.level = webrtc::AudioProcessing::Config::NoiseSuppression::kVeryHigh;

    // Disable AGC1 entirely - it is the primary cause of "pumping" volume
    m_impl->apConfig.gain_controller1.enabled = false; 

    // Use AGC2 for the actual volume leveling
    m_impl->apConfig.gain_controller2.enabled = false;
    m_impl->apConfig.gain_controller2.adaptive_digital.enabled = false;

    webrtc::BuiltinAudioProcessingBuilder builder(m_impl->apConfig);
    m_impl->apm = builder.Build(*m_impl->env);

    if (m_impl->apm) {
        m_impl->apm->ApplyConfig(m_impl->apConfig);
    }
}

//============================================================================
WebRtcAec::~WebRtcAec() = default;

//============================================================================
void WebRtcAec::setStreamFormat( int sample_rate_hz, size_t channels )
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->apm || sample_rate_hz <= 0 || channels == 0) {
        return;
    }

    if (sample_rate_hz == m_impl->sampleRateHz && channels == m_impl->channels) {
        return;
    }

    m_impl->sampleRateHz = sample_rate_hz;
    m_impl->channels = channels;
    m_impl->frameSize = static_cast<size_t>( webrtc::AudioProcessing::GetFrameSize(sample_rate_hz) );

    webrtc::StreamConfig streamConfig( sample_rate_hz, channels );
    m_impl->streamConfig = streamConfig;

    m_impl->processingConfig.input_stream().set_sample_rate_hz(sample_rate_hz);
    m_impl->processingConfig.input_stream().set_num_channels(channels);
    m_impl->processingConfig.output_stream().set_sample_rate_hz(sample_rate_hz);
    m_impl->processingConfig.output_stream().set_num_channels(channels);
    m_impl->processingConfig.reverse_input_stream().set_sample_rate_hz(sample_rate_hz);
    m_impl->processingConfig.reverse_input_stream().set_num_channels(channels);
    m_impl->processingConfig.reverse_output_stream().set_sample_rate_hz(sample_rate_hz);
    m_impl->processingConfig.reverse_output_stream().set_num_channels(channels);

    m_impl->apm->Initialize(m_impl->processingConfig);
}

//============================================================================
void WebRtcAec::setAgcEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_agcEnabled = enabled;

    if (!m_impl->apm) {
        return;
    }

    m_impl->apConfig.gain_controller2.enabled = m_agcEnabled;

    m_impl->apm->ApplyConfig(m_impl->apConfig);
}

//============================================================================
bool WebRtcAec::isAgcEnabled() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->apConfig.gain_controller2.enabled;
}

//============================================================================
void WebRtcAec::setEchoDelay( int delayMs )
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->currentEchoDelayMs = delayMs;
}

//============================================================================
float WebRtcAec::lastVadProbability( void ) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->lastVadProbability;
}

//============================================================================
float WebRtcAec::lastEchoReturnLoss( void ) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->lastEchoReturnLoss;
}

//============================================================================
void WebRtcAec::processCapture( int16_t* data, int frames )
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->apm || !data || frames <= 0 || m_impl->sampleRateHz <= 0 || m_impl->channels == 0) 
    {
        return;
    }

    const size_t frameCount = static_cast<size_t>(frames);
    if (m_impl->frameSize != 0 && frameCount != m_impl->frameSize) 
    {
        return;
    }

    m_impl->apm->set_stream_delay_ms( m_impl->currentEchoDelayMs );

    m_impl->apm->ProcessStream(data, m_impl->streamConfig, m_impl->streamConfig, data);

    const webrtc::AudioProcessingStats stats = m_impl->apm->GetStatistics(true);
    m_impl->lastEchoReturnLoss = stats.echo_return_loss.has_value()
        ? static_cast<float>(*stats.echo_return_loss)
        : 0.0f;
    double vadEstimate = stats.residual_echo_likelihood.value_or(0.0);
    if (vadEstimate < 0.0) {
        vadEstimate = 0.0;
    } else if (vadEstimate > 1.0) {
        vadEstimate = 1.0;
    }
    m_impl->lastVadProbability = static_cast<float>(vadEstimate);
}

//============================================================================
void WebRtcAec::processRender( int16_t* data, int frames )
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->apm || !data || frames <= 0 || m_impl->sampleRateHz <= 0 || m_impl->channels == 0) {
        return;
    }

    const size_t frameCount = static_cast<size_t>(frames);
    if (m_impl->frameSize != 0 && frameCount != m_impl->frameSize) {
        return;
    }

    const size_t sampleCount = frameCount * m_impl->channels;
    if (m_impl->renderScratch.size() < sampleCount) {
        m_impl->renderScratch.resize(sampleCount);
    }

    // this changes the data in the buffer
    m_impl->apm->ProcessReverseStream( data, m_impl->streamConfig, m_impl->streamConfig, data) ;
}
