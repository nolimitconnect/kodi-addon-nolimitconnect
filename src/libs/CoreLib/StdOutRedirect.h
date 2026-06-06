#ifndef STD_OUT_REDIRECT_H
#define STD_OUT_REDIRECT_H
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include <CoreLib/config_corelib.h>

class StdOutRedirect
{
public:
	StdOutRedirect(int bufferSize);
	virtual ~StdOutRedirect();

	int startRedirect();
	int stopRedirect();
	int getStdOutBuffer(char *buffer, int size);
	int getStdErrBuffer(char *buffer, int size);

private:
	int m_fdStdOutPipe[2];
	int m_fdStdOut;
	int m_fdStdErrPipe[2];
	int m_fdStdErr;
};

#endif // STD_OUT_REDIRECT_H
