// Minimal logging stubs for WebRTC APM Phase 2
// Routes logging/checks to CoreLib logger without pulling full rtc_base logging.

#if defined(WEBRTC_WIN)
#include <Winsock2.h>
#include <windows.h>
#define LAST_SYSTEM_ERROR (::GetLastError())
#elif defined(WEBRTC_POSIX)
#include <cerrno>
#define LAST_SYSTEM_ERROR (errno)
#endif  // WEBRTC_WIN

#include <atomic>
#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <string>

#include "CoreLib/Logger.h"
#include "absl/strings/string_view.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/strings/string_builder.h"

namespace webrtc {
namespace {

void LogToCoreLib(LoggingSeverity severity, absl::string_view msg) {
  std::string text(msg);
  switch (severity) {
    case LS_VERBOSE:
      nlc_verbose.log("%s", text.c_str());
      break;
    case LS_INFO:
      nlc_info.log("%s", text.c_str());
      break;
    case LS_WARNING:
      nlc_warn.log("%s", text.c_str());
      break;
    case LS_ERROR:
      nlc_err.log("%s", text.c_str());
      break;
    case LS_NONE:
    default:
      break;
  }
}

}  // namespace

std::string LogLineRef::DefaultLogLine() const {
  return std::string(message_);
}

void LogSink::OnLogMessage(const std::string& msg,
                           LoggingSeverity /* severity */,
                           const char* /* tag */) {
  OnLogMessage(msg);
}

void LogSink::OnLogMessage(const std::string& msg,
                           LoggingSeverity /* severity */) {
  OnLogMessage(msg);
}

void LogSink::OnLogMessage(const std::string& message) {
  (void)message;
}

void LogSink::OnLogMessage(absl::string_view msg,
                           LoggingSeverity /* severity */,
                           const char* /* tag */) {
  OnLogMessage(msg);
}

void LogSink::OnLogMessage(absl::string_view message,
                           LoggingSeverity /* severity */) {
  OnLogMessage(message);
}

void LogSink::OnLogMessage(absl::string_view message) {
  OnLogMessage(std::string(message));
}

void LogSink::OnLogMessage(const LogLineRef& line) {
  OnLogMessage(line.DefaultLogLine());
}

LogSink* LogMessage::streams_ = nullptr;
std::atomic<bool> LogMessage::streams_empty_ = {true};
bool LogMessage::log_thread_ = false;
bool LogMessage::log_timestamp_ = false;
bool LogMessage::log_to_stderr_ = true;

LogMessage::LogMessage(const char* file, int line, LoggingSeverity sev) {
  log_line_.set_severity(sev);
  if (file != nullptr) {
    log_line_.set_filename(file);
    log_line_.set_line(line);
  }
}

LogMessage::LogMessage(const char* file,
                       int line,
                       LoggingSeverity sev,
                       LogErrorContext /* err_ctx */,
                       int /* err */) {
  log_line_.set_severity(sev);
  if (file != nullptr) {
    log_line_.set_filename(file);
    log_line_.set_line(line);
  }
}

#if defined(WEBRTC_ANDROID)
LogMessage::LogMessage(const char* file,
                       int line,
                       LoggingSeverity sev,
                       const char* tag) {
  log_line_.set_severity(sev);
  if (file != nullptr) {
    log_line_.set_filename(file);
    log_line_.set_line(line);
  }
  if (tag != nullptr) {
    log_line_.set_tag(tag);
  }
}
#endif

LogMessage::~LogMessage() {
  FinishPrintStream();
}

void LogMessage::AddTag(const char* tag) {
  if (tag != nullptr) {
    log_line_.set_tag(tag);
  }
}

StringBuilder& LogMessage::stream() {
  return print_stream_;
}

int64_t LogMessage::LogStartTime() {
  return 0;
}

uint32_t LogMessage::WallClockStartTime() {
  return 0;
}

void LogMessage::LogThreads(bool /* on */) {}

void LogMessage::LogTimestamps(bool /* on */) {}

void LogMessage::LogToDebug(LoggingSeverity /* min_sev */) {}

LoggingSeverity LogMessage::GetLogToDebug() {
  return LoggingSeverity::LS_INFO;
}

void LogMessage::SetLogToStderr(bool /* log_to_stderr */) {}

void LogMessage::AddLogToStream(LogSink* /* stream */,
                                LoggingSeverity /* min_sev */) {}

void LogMessage::RemoveLogToStream(LogSink* /* stream */) {}

int LogMessage::GetLogToStream(LogSink* /* stream */) {
  return static_cast<int>(LoggingSeverity::LS_INFO);
}

int LogMessage::GetMinLogSeverity() {
  return static_cast<int>(LoggingSeverity::LS_INFO);
}

void LogMessage::ConfigureLogging(absl::string_view /* params */) {}

bool LogMessage::IsNoop(LoggingSeverity /* severity */) {
  return false;
}

void LogMessage::UpdateMinLogSeverity() {}

void LogMessage::OutputToDebug(const LogLineRef& log_line_ref) {
  LogToCoreLib(log_line_ref.severity(), log_line_ref.message());
}

void LogMessage::FinishPrintStream() {
  std::string message = print_stream_.Release();
  if (message.empty()) {
    return;
  }
  log_line_.set_message(std::move(message));
  LogToCoreLib(log_line_.severity(), log_line_.message());
}

namespace webrtc_logging_impl {

void Log(const LogArgType* fmt, ...) {
  if (fmt == nullptr) {
    LogToCoreLib(LoggingSeverity::LS_INFO, "WebRTC log");
    return;
  }

  va_list args;
  va_start(args, fmt);

  LogMetadataErr meta;
  const char* tag = nullptr;
  switch (*fmt) {
    case LogArgType::kLogMetadata: {
      meta.meta = va_arg(args, LogMetadata);
      meta.err_ctx = ERRCTX_NONE;
      meta.err = 0;
      break;
    }
    case LogArgType::kLogMetadataErr: {
      meta = va_arg(args, LogMetadataErr);
      break;
    }
#if defined(WEBRTC_ANDROID)
    case LogArgType::kLogMetadataTag: {
      const LogMetadataTag tag_meta = va_arg(args, LogMetadataTag);
      meta.meta = LogMetadata(nullptr, 0, tag_meta.severity);
      meta.err_ctx = ERRCTX_NONE;
      meta.err = 0;
      tag = tag_meta.tag;
      break;
    }
#endif
    default: {
      va_end(args);
      LogToCoreLib(LoggingSeverity::LS_WARNING,
                   "WebRTC log: invalid log metadata");
      return;
    }
  }

  StringBuilder builder;
  for (++fmt; *fmt != LogArgType::kEnd; ++fmt) {
    switch (*fmt) {
      case LogArgType::kInt:
        builder << va_arg(args, int);
        break;
      case LogArgType::kLong:
        builder << va_arg(args, long);
        break;
      case LogArgType::kLongLong:
        builder << va_arg(args, long long);
        break;
      case LogArgType::kUInt:
        builder << va_arg(args, unsigned);
        break;
      case LogArgType::kULong:
        builder << va_arg(args, unsigned long);
        break;
      case LogArgType::kULongLong:
        builder << va_arg(args, unsigned long long);
        break;
      case LogArgType::kDouble:
        builder << va_arg(args, double);
        break;
      case LogArgType::kLongDouble:
        builder.AppendFormat("%Lf", va_arg(args, long double));
        break;
      case LogArgType::kCharP: {
        const char* s = va_arg(args, const char*);
        builder << (s ? s : "(null)");
        break;
      }
      case LogArgType::kStdString:
        builder << *va_arg(args, const std::string*);
        break;
      case LogArgType::kStringView:
        builder << *va_arg(args, const absl::string_view*);
        break;
      case LogArgType::kVoidP:
        builder.AppendFormat("%p", va_arg(args, const void*));
        break;
      default:
        va_end(args);
        LogToCoreLib(LoggingSeverity::LS_WARNING,
                     "WebRTC log: invalid log argument type");
        return;
    }
  }

  va_end(args);

  StringBuilder prefixed;
  if (tag != nullptr && *tag != '\0') {
    prefixed << tag << ": ";
  }
  prefixed << builder.str();

  std::string message = prefixed.Release();
  if (message.empty()) {
    message = "WebRTC log";
  }
  LogToCoreLib(meta.meta.Severity(), message);
}

}  // namespace webrtc_logging_impl

namespace webrtc_checks_impl {

namespace {

bool ParseCheckArg(va_list* args,
                   const CheckArgType** fmt,
                   StringBuilder* output) {
  if (fmt == nullptr || *fmt == nullptr || **fmt == CheckArgType::kEnd) {
    return false;
  }

  switch (**fmt) {
    case CheckArgType::kInt:
      output->AppendFormat("%d", va_arg(*args, int));
      break;
    case CheckArgType::kLong:
      output->AppendFormat("%ld", va_arg(*args, long));
      break;
    case CheckArgType::kLongLong:
      output->AppendFormat("%lld", va_arg(*args, long long));
      break;
    case CheckArgType::kUInt:
      output->AppendFormat("%u", va_arg(*args, unsigned));
      break;
    case CheckArgType::kULong:
      output->AppendFormat("%lu", va_arg(*args, unsigned long));
      break;
    case CheckArgType::kULongLong:
      output->AppendFormat("%llu", va_arg(*args, unsigned long long));
      break;
    case CheckArgType::kDouble:
      output->AppendFormat("%g", va_arg(*args, double));
      break;
    case CheckArgType::kLongDouble:
      output->AppendFormat("%Lg", va_arg(*args, long double));
      break;
    case CheckArgType::kCharP: {
      const char* s = va_arg(*args, const char*);
      *output << (s ? s : "(null)");
      break;
    }
    case CheckArgType::kStdString:
      *output << *va_arg(*args, const std::string*);
      break;
    case CheckArgType::kStringView:
      *output << *va_arg(*args, const absl::string_view*);
      break;
    case CheckArgType::kVoidP:
      output->AppendFormat("%p", va_arg(*args, const void*));
      break;
    default:
      *output << "[Invalid CheckArgType]";
      return false;
  }

  (*fmt)++;
  return true;
}

}  // namespace

RTC_NORETURN void WriteFatalLog(const char* file,
                                int line,
                                absl::string_view output) {
  (void)file;
  (void)line;
  WriteFatalLog(output);
}

RTC_NORETURN void WriteFatalLog(absl::string_view output) {
  std::string msg(output);
  nlc_fatal.log("%s", msg.c_str());
  std::abort();
}

void UnreachableCodeReached(const char* file, int line) {
  nlc_fatal.log("%s:%d unreachable", file ? file : "", line);
  std::abort();
}

void UnreachableCodeReached() {
  nlc_fatal.log("unreachable");
  std::abort();
}

RTC_NORETURN void FatalLog(const char* file,
                           int line,
                           const char* message,
                           const CheckArgType* fmt,
                           ...) {
  va_list args;
  va_start(args, fmt);

  StringBuilder output;
  output.AppendFormat("\n\n#\n# Fatal error in: %s, line %d\n# last system error: %u\n# Check failed: %s",
                      file ? file : "", line, LAST_SYSTEM_ERROR,
                      message ? message : "");

  if (fmt != nullptr && *fmt == CheckArgType::kCheckOp) {
    ++fmt;
    StringBuilder lhs;
    StringBuilder rhs;
    if (ParseCheckArg(&args, &fmt, &lhs) && ParseCheckArg(&args, &fmt, &rhs)) {
      output.AppendFormat(" (%s vs. %s)\n# ", lhs.str().c_str(),
                          rhs.str().c_str());
    } else {
      output << "\n# ";
    }
  } else {
    output << "\n# ";
  }

  while (ParseCheckArg(&args, &fmt, &output)) {
  }

  va_end(args);

  nlc_fatal.log("%s", output.str().c_str());
  std::abort();
}

}  // namespace webrtc_checks_impl

}  // namespace webrtc

extern "C" RTC_NORETURN void rtc_FatalMessage(const char* file,
                                              int line,
                                              const char* msg) {
  nlc_fatal.log("%s:%d %s", file ? file : "", line, msg ? msg : "");
  std::abort();
}
