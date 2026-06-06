//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxMemoryUsage.h"

#if defined(TARGET_OS_ANDROID)
#include "VxJava.h"
#elif defined(TARGET_OS_WINDOWS)
#include <windows.h>
#include <Psapi.h>
#else
#include <sys/types.h>
#include <sys/sysinfo.h>
#endif // defined(TARGET_OS_ANDROID)

namespace
{
#if defined(TARGET_OS_ANDROID)
bool GetAndroidHeapSize( JNIEnv* jniEnv, uint64_t& totalMemory )
{
    jclass clazz = jniEnv->FindClass("android/os/Debug");
    if (clazz)
    {
        jmethodID mid = jniEnv->GetStaticMethodID( clazz, "getNativeHeapAllocatedSize", "()J");
        if (mid)
        {
            totalMemory = jniEnv->CallStaticLongMethod( clazz, mid);
            return true;
        }
    }
    return false;
}

bool GetAndroidHeapAllocatedSize(JNIEnv* jniEnv, uint64_t& memoryInUse)
{
    jclass clazz = jniEnv->FindClass( "android/os/Debug");
    if (clazz)
    {
        jmethodID mid = jniEnv->GetStaticMethodID( clazz, "getNativeHeapSize", "()J");
        if (mid)
        {
            memoryInUse = jniEnv->CallStaticLongMethod( clazz, mid);
            return true;
        }
    }
    return false;
}
#endif // defined(TARGET_OS_ANDROID)

} // namespace


bool VxGetMemoryUsage( uint64_t& memoryInUse, uint64_t& totalMemory )
{
#if defined(TARGET_OS_ANDROID)
    JNIEnv* jniEnv = GetJavaEnvCache().getJavaEnv();
    if( jniEnv )
    {
        return GetAndroidHeapSize( jniEnv, memoryInUse ) && GetAndroidHeapAllocatedSize( jniEnv, totalMemory );
    }
    return false;
#elif defined(TARGET_OS_WINDOWS)

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof( MEMORYSTATUSEX );
    if( GlobalMemoryStatusEx( &memInfo ) )
    {
        totalMemory = memInfo.ullTotalPageFile;
        memoryInUse = totalMemory - memInfo.ullAvailPageFile;
        return true;
    }
    else
    {
        return false;
    }

#else

struct sysinfo memInfo;

sysinfo (&memInfo);
totalMemory = memInfo.totalram;
//Add other values in next statement to avoid int overflow on right hand side...
totalMemory += memInfo.totalswap;
totalMemory *= memInfo.mem_unit;

memoryInUse = memInfo.totalram - memInfo.freeram;
//Add other values in next statement to avoid int overflow on right hand side...
memoryInUse += memInfo.totalswap - memInfo.freeswap;
memoryInUse *= memInfo.mem_unit;
return true;

#endif // defined(TARGET_OS_ANDROID)

}
