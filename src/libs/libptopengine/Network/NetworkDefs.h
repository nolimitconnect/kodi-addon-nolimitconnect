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

#define UDP_ANNOUNCE_BROADCAST_INTERVAL			4

#define DIRECT_CONNECT_TIMEOUT					7000
#define LAN_CONNECT_TIMEOUT						5000
#define MY_PROXY_CONNECT_TIMEOUT				7000
#define TO_PROXY_CONNECT_TIMEOUT				7000
#define THROUGH_PROXY_RESPONSE_TIMEOUT			12000
#define TRY_CONNECT_TO_PROXY_SECONDS			10 // try to connect to proxy interval

#define NETSERVICE_RX_URL_HDR_TIMEOUT			30000
#define NETSERVICE_RX_DATA_TIMEOUT				20000
#define IS_PORT_OPEN_RX_HDR_TIMEOUT				30000
#define IS_PORT_OPEN_RX_DATA_TIMEOUT			30000
#define QUERY_HOST_ID_RX_HDR_TIMEOUT			8000
#define QUERY_HOST_ID_RX_DATA_TIMEOUT			5000
#define PING_TEST_RX_HDR_TIMEOUT			    6000
#define PING_TEST_RX_DATA_TIMEOUT			    5000

#define PORT_TEST_CONNECT_TO_CLIENT1_TIMEOUT	5000
#define PONG_RX_TIMEOUT							6000
#define NETSERVICE_TX_TIMEOUT					2000

