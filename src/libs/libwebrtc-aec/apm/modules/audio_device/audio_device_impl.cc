/*
 *  Copyright (c) 2024 The nlc_voip_test Project Authors. All Rights Reserved.
 *
 *  Minimal stub implementation of AudioDeviceModule for use with vendored
 *  WebRTC Audio Processing Module without platform-specific audio device code.
 */

#include "modules/audio_device/audio_device_impl.h"

#include <cstring>

namespace webrtc {

scoped_refptr<AudioDeviceModule> AudioDeviceModuleImpl::Create(
    const Environment& env,
    AudioDeviceModule::AudioLayer audio_layer) {
  RefCountedObject<AudioDeviceModuleImpl>* impl =
      new RefCountedObject<AudioDeviceModuleImpl>();
  impl->active_audio_layer_ = audio_layer;
  // Return through base class pointer
  return scoped_refptr<AudioDeviceModule>(
      static_cast<AudioDeviceModule*>(impl));
}

int32_t AudioDeviceModuleImpl::ActiveAudioLayer(AudioLayer* audioLayer) const {
  if (!audioLayer) return -1;
  *audioLayer = active_audio_layer_;
  return 0;
}

int32_t AudioDeviceModuleImpl::RegisterAudioCallback(
    AudioTransport* audioCallback) {
  audio_transport_callback_ = audioCallback;
  return 0;
}

int32_t AudioDeviceModuleImpl::Init() {
  initialized_ = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::Terminate() {
  initialized_ = false;
  recording_ = false;
  playing_ = false;
  return 0;
}

bool AudioDeviceModuleImpl::Initialized() const {
  return initialized_;
}

int16_t AudioDeviceModuleImpl::PlayoutDevices() {
  return 1;
}

int16_t AudioDeviceModuleImpl::RecordingDevices() {
  return 1;
}

int32_t AudioDeviceModuleImpl::PlayoutDeviceName(uint16_t index,
                                               char name[kAdmMaxDeviceNameSize],
                                               char guid[kAdmMaxGuidSize]) {
  if (index != 0) return -1;
  if (name) strncpy(name, "Default Playout", kAdmMaxDeviceNameSize - 1);
  if (guid) guid[0] = '\0';
  return 0;
}

int32_t AudioDeviceModuleImpl::RecordingDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  if (index != 0) return -1;
  if (name) strncpy(name, "Default Recording", kAdmMaxDeviceNameSize - 1);
  if (guid) guid[0] = '\0';
  return 0;
}

int32_t AudioDeviceModuleImpl::SetPlayoutDevice(uint16_t index) {
  return (index == 0) ? 0 : -1;
}

int32_t AudioDeviceModuleImpl::SetPlayoutDevice(WindowsDeviceType device) {
  return 0;
}

int32_t AudioDeviceModuleImpl::SetRecordingDevice(uint16_t index) {
  return (index == 0) ? 0 : -1;
}

int32_t AudioDeviceModuleImpl::SetRecordingDevice(WindowsDeviceType device) {
  return 0;
}

int32_t AudioDeviceModuleImpl::PlayoutIsAvailable(bool* available) {
  if (!available) return -1;
  *available = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::InitPlayout() {
  return 0;
}

bool AudioDeviceModuleImpl::PlayoutIsInitialized() const {
  return true;
}

int32_t AudioDeviceModuleImpl::RecordingIsAvailable(bool* available) {
  if (!available) return -1;
  *available = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::InitRecording() {
  return 0;
}

bool AudioDeviceModuleImpl::RecordingIsInitialized() const {
  return true;
}

int32_t AudioDeviceModuleImpl::StartPlayout() {
  if (!initialized_) return -1;
  playing_ = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::StopPlayout() {
  playing_ = false;
  return 0;
}

bool AudioDeviceModuleImpl::Playing() const {
  return playing_;
}

int32_t AudioDeviceModuleImpl::StartRecording() {
  if (!initialized_) return -1;
  recording_ = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::StopRecording() {
  recording_ = false;
  return 0;
}

bool AudioDeviceModuleImpl::Recording() const {
  return recording_;
}

int32_t AudioDeviceModuleImpl::InitSpeaker() {
  return 0;
}

bool AudioDeviceModuleImpl::SpeakerIsInitialized() const {
  return true;
}

int32_t AudioDeviceModuleImpl::InitMicrophone() {
  return 0;
}

bool AudioDeviceModuleImpl::MicrophoneIsInitialized() const {
  return true;
}

int32_t AudioDeviceModuleImpl::SpeakerVolumeIsAvailable(bool* available) {
  if (!available) return -1;
  *available = false;
  return 0;
}

int32_t AudioDeviceModuleImpl::SetSpeakerVolume(uint32_t volume) {
  return 0;
}

int32_t AudioDeviceModuleImpl::SpeakerVolume(uint32_t* volume) const {
  if (!volume) return -1;
  *volume = 255;
  return 0;
}

int32_t AudioDeviceModuleImpl::MaxSpeakerVolume(uint32_t* maxVolume) const {
  if (!maxVolume) return -1;
  *maxVolume = 255;
  return 0;
}

int32_t AudioDeviceModuleImpl::MinSpeakerVolume(uint32_t* minVolume) const {
  if (!minVolume) return -1;
  *minVolume = 0;
  return 0;
}

int32_t AudioDeviceModuleImpl::MicrophoneVolumeIsAvailable(bool* available) {
  if (!available) return -1;
  *available = false;
  return 0;
}

int32_t AudioDeviceModuleImpl::SetMicrophoneVolume(uint32_t volume) {
  return 0;
}

int32_t AudioDeviceModuleImpl::MicrophoneVolume(uint32_t* volume) const {
  if (!volume) return -1;
  *volume = 255;
  return 0;
}

int32_t AudioDeviceModuleImpl::MaxMicrophoneVolume(uint32_t* maxVolume) const {
  if (!maxVolume) return -1;
  *maxVolume = 255;
  return 0;
}

int32_t AudioDeviceModuleImpl::MinMicrophoneVolume(uint32_t* minVolume) const {
  if (!minVolume) return -1;
  *minVolume = 0;
  return 0;
}

int32_t AudioDeviceModuleImpl::SpeakerMuteIsAvailable(bool* available) {
  if (!available) return -1;
  *available = false;
  return 0;
}

int32_t AudioDeviceModuleImpl::SetSpeakerMute(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::SpeakerMute(bool* enabled) const {
  if (!enabled) return -1;
  *enabled = false;
  return 0;
}

int32_t AudioDeviceModuleImpl::MicrophoneMuteIsAvailable(bool* available) {
  if (!available) return -1;
  *available = false;
  return 0;
}

int32_t AudioDeviceModuleImpl::SetMicrophoneMute(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::MicrophoneMute(bool* enabled) const {
  if (!enabled) return -1;
  *enabled = false;
  return 0;
}

int32_t AudioDeviceModuleImpl::StereoPlayoutIsAvailable(bool* available) const {
  if (!available) return -1;
  *available = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::SetStereoPlayout(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::StereoPlayout(bool* enabled) const {
  if (!enabled) return -1;
  *enabled = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::StereoRecordingIsAvailable(
    bool* available) const {
  if (!available) return -1;
  *available = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::SetStereoRecording(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::StereoRecording(bool* enabled) const {
  if (!enabled) return -1;
  *enabled = true;
  return 0;
}

int32_t AudioDeviceModuleImpl::PlayoutDelay(uint16_t* delayMS) const {
  if (!delayMS) return -1;
  *delayMS = 50;
  return 0;
}

bool AudioDeviceModuleImpl::BuiltInAECIsAvailable() const {
  return false;
}

bool AudioDeviceModuleImpl::BuiltInAGCIsAvailable() const {
  return false;
}

bool AudioDeviceModuleImpl::BuiltInNSIsAvailable() const {
  return false;
}

int32_t AudioDeviceModuleImpl::EnableBuiltInAEC(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::EnableBuiltInAGC(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::EnableBuiltInNS(bool enable) {
  return 0;
}

int32_t AudioDeviceModuleImpl::GetPlayoutUnderrunCount() const {
  return 0;
}

std::optional<AudioDeviceModule::Stats> AudioDeviceModuleImpl::GetStats()
    const {
  AudioDeviceModule::Stats stats;
  return stats;
}

#if defined(WEBRTC_IOS)
int AudioDeviceModuleImpl::GetPlayoutAudioParameters(
    AudioParameters* params) const {
  return -1;
}

int AudioDeviceModuleImpl::GetRecordAudioParameters(
    AudioParameters* params) const {
  return -1;
}
#endif  // WEBRTC_IOS

}  // namespace webrtc
