/*
 *  Stub implementation - TensorFlow Lite not available
 */

#include "modules/audio_processing/aec3/neural_residual_echo_estimator/neural_residual_echo_estimator_impl.h"

namespace webrtc {

std::unique_ptr<NeuralResidualEchoEstimator>
NeuralResidualEchoEstimatorImpl::Create(const void* model,
                                       const void& op_resolver) {
  // Neural estimation disabled (TFLite not available)
  return nullptr;
}

}  // namespace webrtc
