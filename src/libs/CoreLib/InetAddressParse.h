// assummed pubic domain code from https://rosettacode.org/wiki/Parse_an_IP_Address#C

#include <stdint.h>
#include <string>

bool ParseIPv4OrIPv6( const char* ipAddr, unsigned char* retAddrBuf, uint16_t& retPort, bool& retIsIPv6 );

bool IsValidIpAddr( const char* ipAddr );

void TestsParseIp( void );
