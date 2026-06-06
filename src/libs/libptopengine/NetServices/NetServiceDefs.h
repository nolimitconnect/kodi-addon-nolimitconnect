#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <PktLib/VxCommon.h>

#define NET_SERVICE_HDR_LEN 96

#define NET_CMD_UNKNOWN				"CMD_UNKNOWN        "
#define NET_CMD_HOST_PING			"CMD_HOST_PING      "
#define NET_CMD_HOST_PONG			"CMD_HOST_PONG      "
#define NET_CMD_CLIENT_PING			"CMD_CLIENT_PING    "
#define NET_CMD_CLIENT_PONG			"CMD_CLIENT_PONG    "
#define NET_CMD_PORT_TEST_REQ		"CMD_PORT_TEST_REQ  "
#define NET_CMD_PORT_TEST_REPLY		"CMD_PORT_TEST_REPLY"
#define NET_CMD_HOST_ID_REQ		    "CMD_HOST_ID_REQ    "
#define NET_CMD_HOST_ID_REPLY		"CMD_HOST_ID_REPLY  "
#define NET_CMD_HOST_PING_REQ		"CMD_HOST_PING_REQ  "
#define NET_CMD_HOST_PING_REPLY		"CMD_HOST_PING_REPLY"

constexpr int MAX_NET_CMD_CONTENT_LEN = 64;
constexpr int MIN_NET_CMD_LEN = NET_SERVICE_HDR_LEN;
constexpr int MAX_NET_CMD_LEN = NET_SERVICE_HDR_LEN + MAX_NET_CMD_CONTENT_LEN;


