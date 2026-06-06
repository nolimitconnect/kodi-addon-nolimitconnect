#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#ifdef TARGET_OS_WINDOWS

#include <CoreLib/config_corelib.h>

#include <WinSock2.h>
#include <windows.h>
#include <string>

HRESULT VxCreateShortcut(	std::wstring csLinkName, //Full path to link ( link name should not have the .lnk extention )
												//example C:\\windows\\Desktop\\MyApp
							std::wstring csPathToExe, //path to exe to run when link is run
							std::wstring csLinkDesc	); //description of link

HRESULT VxResolveShortcut(	HWND hWnd,				//handle to window of caller
							std::wstring lpszLinkFile,	//.lnk file
							std::wstring& lpszPath );		//return path to target file
#endif // TARGET_OS_WINDOWS
