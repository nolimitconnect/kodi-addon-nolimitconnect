/*
 *  Stub FieldTrialsRegistry implementation without registered field trials.
 */

#include "api/field_trials_registry.h"

namespace webrtc {

std::string FieldTrialsRegistry::Lookup(absl::string_view key) const {
  return GetValue(key);
}

}  // namespace webrtc
