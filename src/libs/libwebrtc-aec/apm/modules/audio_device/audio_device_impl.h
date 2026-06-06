/*
 *  Copyright (c) 2024 The nlc_voip_test Project Authors. All Rights Reserved.
 *
 *  Minimal stub implementation of AudioDeviceModule for use with vendored
 *  WebRTC Audio Processing Module without platform-specific audio device code.
 */

#ifndef MODULES_AUDIO_DEVICE_AUDIO_DEVICE_IMPL_H_
#define MODULES_AUDIO_DEVICE_AUDIO_DEVICE_IMPL_H_

#include "api/audio/audio_device.h"
#include "api/environment/environment.h"
#include "api/ref_count.h"
#include "rtc_base/ref_counted_object.h"

namespace webrtc {

class AudioDeviceModuleImpl : public AudioDeviceModule {
 public:
  // Factory method (required by create_audio_device_module.cc)
  static scoped_refptr<AudioDeviceModule> Create(const Environment& env,
                                                   AudioLayer audioLayer);

  // Retrieve the currently utilized audio layer
  int32_t ActiveAudioLayer(AudioLayer* audioLayer) const override;

  // Full-duplex transportation of PCM audio
  int32_t RegisterAudioCallback(AudioTransport* audioCallback) override;

  // Main initialization and termination
  int32_t Init() override;
  int32_t Terminate() override;
  bool Initialized() const override;

  // Device enumeration
  int16_t PlayoutDevices() override;
  int16_t RecordingDevices() override;
  int32_t PlayoutDeviceName(uint16_t index,
                            char name[kAdmMaxDeviceNameSize],
                            char guid[kAdmMaxGuidSize]) override;
  int32_t RecordingDeviceName(uint16_t index,
                              char name[kAdmMaxDeviceNameSize],
                              char guid[kAdmMaxGuidSize]) override;

  // Device selection
  int32_t SetPlayoutDevice(uint16_t index) override;
  int32_t SetPlayoutDevice(WindowsDeviceType device) override;
  int32_t SetRecordingDevice(uint16_t index) override;
  int32_t SetRecordingDevice(WindowsDeviceType device) override;

  // Audio transport initialization
  int32_t PlayoutIsAvailable(bool* available) override;
  int32_t InitPlayout() override;
  bool PlayoutIsInitialized() const override;
  int32_t RecordingIsAvailable(bool* available) override;
  int32_t InitRecording() override;
  bool RecordingIsInitialized() const override;

  // Audio transport control
  int32_t StartPlayout() override;
  int32_t StopPlayout() override;
  bool Playing() const override;
  int32_t StartRecording() override;
  int32_t StopRecording() override;
  bool Recording() const override;

  // Audio mixer initialization
  int32_t InitSpeaker() override;
  bool SpeakerIsInitialized() const override;
  int32_t InitMicrophone() override;
  bool MicrophoneIsInitialized() const override;

  // Speaker volume controls
  int32_t SpeakerVolumeIsAvailable(bool* available) override;
  int32_t SetSpeakerVolume(uint32_t volume) override;
  int32_t SpeakerVolume(uint32_t* volume) const override;
  int32_t MaxSpeakerVolume(uint32_t* maxVolume) const override;
  int32_t MinSpeakerVolume(uint32_t* minVolume) const override;

  // Microphone volume controls
  int32_t MicrophoneVolumeIsAvailable(bool* available) override;
  int32_t SetMicrophoneVolume(uint32_t volume) override;
  int32_t MicrophoneVolume(uint32_t* volume) const override;
  int32_t MaxMicrophoneVolume(uint32_t* maxVolume) const override;
  int32_t MinMicrophoneVolume(uint32_t* minVolume) const override;

  // Speaker mute control
  int32_t SpeakerMuteIsAvailable(bool* available) override;
  int32_t SetSpeakerMute(bool enable) override;
  int32_t SpeakerMute(bool* enabled) const override;

  // Microphone mute control
  int32_t MicrophoneMuteIsAvailable(bool* available) override;
  int32_t SetMicrophoneMute(bool enable) override;
  int32_t MicrophoneMute(bool* enabled) const override;

  // Stereo support
  int32_t StereoPlayoutIsAvailable(bool* available) const override;
  int32_t SetStereoPlayout(bool enable) override;
  int32_t StereoPlayout(bool* enabled) const override;
  int32_t StereoRecordingIsAvailable(bool* available) const override;
  int32_t SetStereoRecording(bool enable) override;
  int32_t StereoRecording(bool* enabled) const override;

  // Playout delay
  int32_t PlayoutDelay(uint16_t* delayMS) const override;

  // Android-specific built-in audio effects availability
  bool BuiltInAECIsAvailable() const override;
  bool BuiltInAGCIsAvailable() const override;
  bool BuiltInNSIsAvailable() const override;

  // Enable built-in audio effects
  int32_t EnableBuiltInAEC(bool enable) override;
  int32_t EnableBuiltInAGC(bool enable) override;
  int32_t EnableBuiltInNS(bool enable) override;

  // Play underrun count (Android)
  int32_t GetPlayoutUnderrunCount() const override;

  // Statistics
  std::optional<Stats> GetStats() const override;

#if defined(WEBRTC_IOS)
  int GetPlayoutAudioParameters(AudioParameters* params) const override;
  int GetRecordAudioParameters(AudioParameters* params) const override;
#endif  // WEBRTC_IOS

 protected:
  AudioDeviceModuleImpl() = default;
  ~AudioDeviceModuleImpl() override = default;

  bool initialized_ = false;
  bool recording_ = false;
  bool playing_ = false;
  AudioTransport* audio_transport_callback_ = nullptr;
  AudioLayer active_audio_layer_ = kPlatformDefaultAudio;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_AUDIO_DEVICE_IMPL_H_
