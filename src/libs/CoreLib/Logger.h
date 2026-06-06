#pragma once
//============================================================================
// Copyright (C) 2026 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <sstream>
#include <mutex>
#include <cstdarg>
#include <cstdio>

#include <CoreLib/VxDebug.h>

// Standard C Linkage for your LogMsg function
//extern "C" void LogMsg(uint32_t u32MsgType, const char* msg, ...);

class NlcLogger {
public:
    explicit NlcLogger(uint32_t type) : m_type(type) {}

    struct LogProxy : public std::ostringstream {
        NlcLogger& m_parent;
        std::unique_lock<std::mutex> m_lock;

        // 1. Standard constructor
        LogProxy(NlcLogger& parent)
            : m_parent(parent), m_lock(parent.m_mutex) {}

        // 2. MOVE CONSTRUCTOR: Required to return by value
        LogProxy(LogProxy&& other) noexcept
            : std::ostringstream(std::move(other)),
            m_parent(other.m_parent),
            m_lock(std::move(other.m_lock)) {}

        // 3. EXPLICITLY DELETE COPY: Prevents the C2280 error
        LogProxy(const LogProxy&) = delete;
        LogProxy& operator=(const LogProxy&) = delete;

        ~LogProxy() {
            // Only log if this instance still "owns" the lock (hasn't been moved)
            if (m_lock.owns_lock()) {
                std::string s = str();
                if (!s.empty()) {
                    if (s.back() == '\n') s.pop_back();
                    LogMsg(m_parent.m_type, "%s", s.c_str());
                }
            }
        }

        // Support manipulators like std::endl
        LogProxy& operator<<(std::ostream& (*manip)(std::ostream&)) {
            manip(*this);
            return *this;
        }

        // Support for data types
        template<typename T>
        LogProxy& operator<<(const T& val) {
            static_cast<std::ostringstream&>(*this) << val;
            return *this;
        }
    };

    // 4. FIX: This operator now creates the proxy and returns it.
    // The compiler will use "Move Elision" or the Move Constructor.
    template<typename T>
    LogProxy operator<<(const T& val) {
        LogProxy proxy(*this); // Construct fresh from parent
        proxy << val;
        return proxy; // Move out
    }


    void log(const char* fmt, ...) {
        if(LogEnabled(eLogWebRtc) == false) 
        {
            return;
        }

        char buf[1024]; // Standardize buffer size
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        std::lock_guard<std::mutex> lock(m_mutex);
        LogModule(eLogWebRtc, m_type, "%s", buf);
    }

private:
    uint32_t m_type;
    std::mutex m_mutex;
};

// Thread-safe Global Instances (C++17 inline prevents ODR violations in static libs)
inline NlcLogger nlc_verbose(LOG_VERBOSE);
inline NlcLogger nlc_fatal(LOG_FATAL);
inline NlcLogger nlc_severe(LOG_SEVERE);
inline NlcLogger nlc_assert(LOG_ASSERT);
inline NlcLogger nlc_err(LOG_ERROR);
inline NlcLogger nlc_warn(LOG_WARN);
inline NlcLogger nlc_debug(LOG_DEBUG);
inline NlcLogger nlc_info(LOG_INFO);
inline NlcLogger nlc_status(LOG_STATUS);

#endif // __cplusplus

