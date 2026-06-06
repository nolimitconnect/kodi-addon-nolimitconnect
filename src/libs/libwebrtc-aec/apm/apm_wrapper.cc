// Minimal APM wrapper providing basic audio processing functionality
// This is a pragmatic interim solution until full APM integration is complete

#include <cstring>

namespace webrtc {

// Minimal APM stub implementation - will be enhanced incrementally

class AudioProcessingImpl {
 public:
  AudioProcessingImpl() = default;
  ~AudioProcessingImpl() = default;
  
  // Stub functions to satisfy linker while real implementations are added
  int ProcessStream() { return 0; }
  int ProcessReverseStream() { return 0; }
};

}  // namespace webrtc

// Entry point for APM library
extern "C" void* CreateAudioProcessing() {
  return new webrtc::AudioProcessingImpl();
}

extern "C" void DestroyAudioProcessing(void* apm) {
  delete static_cast<webrtc::AudioProcessingImpl*>(apm);
}
