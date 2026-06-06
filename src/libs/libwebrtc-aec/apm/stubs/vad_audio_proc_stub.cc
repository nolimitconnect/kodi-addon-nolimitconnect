/*
 *  Stub VadAudioProc implementation to avoid iSAC dependencies.
 */

#include "modules/audio_processing/vad/vad_audio_proc.h"

#include <cmath>

#include "modules/audio_processing/vad/pole_zero_filter.h"
#include "rtc_base/checks.h"

namespace webrtc {

struct VadAudioProc::PitchAnalysisStruct {};
struct VadAudioProc::PreFiltBankstr {};

VadAudioProc::VadAudioProc() = default;
VadAudioProc::~VadAudioProc() = default;

int VadAudioProc::ExtractFeatures(const int16_t* audio_frame,
                                  size_t length,
                                  AudioFeatures* audio_features) {
  if (audio_features == nullptr || audio_frame == nullptr) {
    return -1;
  }

  audio_features->num_frames = 0;
  audio_features->silence = true;

  if (length != kLength10Ms) {
    return -1;
  }

  double sum_sq = 0.0;
  for (size_t i = 0; i < length; ++i) {
    const double sample = static_cast<double>(audio_frame[i]);
    sum_sq += sample * sample;
  }

  const double rms = std::sqrt(sum_sq / static_cast<double>(length));
  audio_features->rms[0] = rms;
  audio_features->log_pitch_gain[0] = 0.0;
  audio_features->pitch_lag_hz[0] = 0.0;
  audio_features->spectral_peak[0] = 0.0;
  audio_features->num_frames = 1;
  audio_features->silence = rms < 1.0;

  return 0;
}

}  // namespace webrtc
