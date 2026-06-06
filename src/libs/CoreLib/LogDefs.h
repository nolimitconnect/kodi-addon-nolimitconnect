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

#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARNING
#undef LOG_CRIT

#define LOG_NONE    (0x0000)
#define LOG_VERBOSE (0x0001)
#define LOG_FATAL	(0x0002)
#define LOG_SEVERE	(0x0004)
#define LOG_ASSERT	(0x0008)
#define LOG_ERROR	(0x0010)
#define LOG_WARN	(0x0020)
#define LOG_DEBUG	(0x0040)
#define LOG_INFO	(0x0080)
#define LOG_STATUS	(0x0100)
#define LOG_PRIORITY_MASK	    0x000001ff

// defines so less work converting Linux code
#define LOG_WARNING		LOG_WARN
#define LOG_CRIT		LOG_FATAL
