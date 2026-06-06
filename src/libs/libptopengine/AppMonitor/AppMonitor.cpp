//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <CoreLib/VxDebug.h>

#ifdef TARGET_OS_WINDOWS
#include <WinSock2.h>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <fstream>
#include <string>
#endif

namespace AppMonitor
{
    void getAppStatus( unsigned long& threadCount, double& rssMB, double& vmsMB );

    void dumpAppStats( void )
    {
        unsigned long threadCount = 0;
        double rssMB = 0.0;
        double vmsMB = 0.0;
        getAppStatus( threadCount, rssMB, vmsMB );
        LogMsg( LOG_DEBUG, "App: threads %d rss memory %3.1f MB vms memory %3.1f MB ", threadCount, rssMB, vmsMB );
    }

    void getAppStatus( unsigned long& threadCount, double& rssMB, double& vmsMB )
    {

#ifdef TARGET_OS_WINDOWS
        // --- Windows implementation ---
        DWORD pid = GetCurrentProcessId();
        HANDLE hProcess = GetCurrentProcess();

        // Thread count
        HANDLE hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof( PROCESSENTRY32 );
        if( Process32First( hSnap, &pe32 ) ) 
        {
            do 
            {
                if( pe32.th32ProcessID == pid ) 
                {
                    threadCount = pe32.cntThreads;
                    break;
                }
            } while( Process32Next( hSnap, &pe32 ) );
        }

        CloseHandle( hSnap );

        // Memory usage
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if( GetProcessMemoryInfo( hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof( pmc ) ) ) {
            rssMB = pmc.WorkingSetSize / ( 1024.0 * 1024.0 );
            vmsMB = pmc.PrivateUsage / ( 1024.0 * 1024.0 );
        }

        CloseHandle( hProcess );

#else
        // --- Linux implementation ---
        pid_t pid = getpid();
        std::string statusPath = "/proc/" + std::to_string( pid ) + "/status";
        std::ifstream statusFile( statusPath );
        std::string line;

        while( std::getline( statusFile, line ) ) 
        {
            if( line.rfind( "Threads:", 0 ) == 0 ) 
            {
                threadCount = std::stoul( line.substr( 8 ) );
            }
            else if( line.rfind( "VmRSS:", 0 ) == 0 ) 
            {
                rssMB = std::stol( line.substr( 6 ) ) / 1024.0;
            }
            else if( line.rfind( "VmSize:", 0 ) == 0 ) 
            {
                vmsMB = std::stol( line.substr( 7 ) ) / 1024.0;
            }
        }
#endif
    }
}
