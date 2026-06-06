// Stub implementations for rtc_base checking/logging infrastructure
#include "rtc_base/logging.h"
#include "rtc_base/checks.h"
#include <cstdarg>
#include <iostream>
#include <cstring>

namespace webrtc {

// For checks.h - FatalLog stub
namespace webrtc_checks_impl {
void FatalLog(const char* file, int line, const char* condition,
              const CheckArgType* args, ...) {
  // Just print to stderr and abort
  std::cerr << "FATAL CHECK: " << file << ":" << line << " - " << condition << std::endl;
  std::abort();
}

void UnreachableCodeReached(const char* file, int line) {
  std::cerr << "UNREACHABLE CODE: " << file << ":" << line << std::endl;
  std::abort();
}
}  // namespace webrtc_checks_impl

// For logging.h - Log stub
namespace webrtc_logging_impl {
void Log(const LogArgType* args, ...) {
  // Stub - do nothing with logging
}
}  // namespace webrtc_logging_impl

// LogMessage stub
bool LogMessage::IsNoop(LoggingSeverity severity) {
  return true;  // Suppress all logs
}

// Metrics stubs - forward declare as opaque type
struct Histogram {};

namespace metrics {
Histogram* HistogramFactoryGetCounts(std::string_view name,
                                      int min, int max, int bucket_count) {
  return nullptr;  // Return null histogram
}

void HistogramAdd(Histogram* histogram, int sample) {
  // Stub - do nothing
}
}  // namespace metrics

}  // namespace webrtc
