#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/config_corelib.h>
#include <string>
#include <time.h>

class VxTimeUtil
{
public:

	static std::string			getFileNameCompatibleDate( int64_t timeGmtMs );
	static std::string			getFileNameCompatibleDateAndTime( int64_t timeGmtMs );

	static std::string			getLocalTime( void );
	static std::string			getChatHourMinTimeStamp( void );

	static std::string			getLocalDateAndTimeWithTextMonths( void );
	static std::string			getLocalDateAndTimeWithNumberMonths( void );
	// shortest time ie. if less than a day ago will be just hour and minute. if more than a year ago then full HH:MM MM/DD/YYYY
	static std::string	        getShortestTime( int64_t timeGmtMs, bool localTime = true );

	// puts http formated time in retBuf.. returns length
	static int					getHttpDateTime( int64_t timeSinceJan1970GmtMs, char * retBuf );
	static std::string			formatTimeStampIntoHoursAndMinutes( int64_t timeSinceJan1970GmtMs );
    static std::string          formatGmtTimeStampIntoHoursAndMinutes( int64_t timeSinceJan1970GmtMs );
	static std::string			formatTimeStampIntoHoursAndMinutesAndSeconds( int64_t timeSinceJan1970GmtMs );
    static std::string			formatGmtTimeStampIntoHoursAndMinutesAndSeconds( int64_t timeSinceJan1970GmtMs );
    static std::string			formatTimeStampIntoDateAndTimeWithTextMonths( int64_t timeSinceJan1970GmtMs );
    static std::string			formatTimeStampIntolDateAndTimeWithNumberMonths( int64_t timeSinceJan1970GmtMs );
	static std::string			formatTimeDiffIntoMinutesAndSeconds( int64_t timeDifMs );
	static void					splitIntoHoursMinutesSeconds( int64_t timeDifMs, int& hrs, int& min, int& sec );
};
