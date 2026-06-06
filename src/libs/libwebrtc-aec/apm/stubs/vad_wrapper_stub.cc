/*
 *  Copyright (c) 2026.
 *
 *  Stub VoiceActivityDetectorWrapper implementation that avoids RNNoise
 *  dependencies while keeping AGC2 functional.
 */

#include "modules/audio_processing/agc2/vad_wrapper.h"

#include <memory>
#include <utility>

#include "modules/audio_processing/agc2/agc2_common.h"
#include "rtc_base/checks.h"

namespace webrtc {
namespace {

constexpr int kNumFramesPerSecond = 100;

class MonoVadImpl : public VoiceActivityDetectorWrapper::MonoVad {
 public:
  explicit MonoVadImpl(int sample_rate_hz) : sample_rate_hz_(sample_rate_hz) {}
  MonoVadImpl(const MonoVadImpl&) = delete;
  MonoVadImpl& operator=(const MonoVadImpl&) = delete;
  ~MonoVadImpl() override = default;

  int SampleRateHz() const override { return sample_rate_hz_; }
  void Reset() override {}
  float Analyze(MonoView<const float> /* frame */) override { return 0.0f; }

 private:
  int sample_rate_hz_;
};

}  // namespace

VoiceActivityDetectorWrapper::VoiceActivityDetectorWrapper(
    const AvailableCpuFeatures& /* cpu_features */,
    int sample_rate_hz)
    : VoiceActivityDetectorWrapper(kVadResetPeriodMs,
                                   std::make_unique<MonoVadImpl>(sample_rate_hz),
                                   sample_rate_hz) {}

VoiceActivityDetectorWrapper::VoiceActivityDetectorWrapper(
    int vad_reset_period_ms,
    const AvailableCpuFeatures& /* cpu_features */,
    int sample_rate_hz)
    : VoiceActivityDetectorWrapper(vad_reset_period_ms,
                                   std::make_unique<MonoVadImpl>(sample_rate_hz),
                                   sample_rate_hz) {}

VoiceActivityDetectorWrapper::VoiceActivityDetectorWrapper(
    int vad_reset_period_ms,
    std::unique_ptr<MonoVad> vad,
    int sample_rate_hz)
    : vad_reset_period_frames_(vad_reset_period_ms / kFrameDurationMs),
      frame_size_(sample_rate_hz / kNumFramesPerSecond),
      time_to_vad_reset_(vad_reset_period_frames_),
      vad_(std::move(vad)),
      resampled_buffer_(vad_->SampleRateHz() / kNumFramesPerSecond),
      resampler_(frame_size_, resampled_buffer_.size(), /* num_channels */ 1) {
  RTC_DCHECK_GT(vad_reset_period_frames_, 1);
  vad_->Reset();
}

VoiceActivityDetectorWrapper::~VoiceActivityDetectorWrapper() = default;

float VoiceActivityDetectorWrapper::Analyze(DeinterleavedView<const float> frame) {
  time_to_vad_reset_--;
  if (time_to_vad_reset_ <= 0) {
    vad_->Reset();
    time_to_vad_reset_ = vad_reset_period_frames_;
  }

  RTC_DCHECK_EQ(frame.samples_per_channel(), frame_size_);
  MonoView<float> dst(resampled_buffer_.data(), resampled_buffer_.size());
  resampler_.Resample(frame[0], dst);

  return vad_->Analyze(resampled_buffer_);
}

}  // namespace webrtc
