//============================================================================
// Copyright (C) 2023 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxMemUtils.h"

#include "VxDebug.h"

#include <malloc.h>

#if defined(TARGET_OS_WINDOWS)
# include <windows.h>
#elif defined(TARGET_OS_ANDROID)
# include <stdlib.h>
#else
# include <cstdlib>
# include <fstream>
#endif // defined(TARGET_OS_WINDOWS)

//============================================================================
void* VxAlignedMalloc( size_t memSize, size_t alignBitCount )
{
#if defined(TARGET_OS_WINDOWS)
    return _aligned_malloc( memSize, alignBitCount );

#elif defined(TARGET_OS_ANDROID)
    void* alignedPtr = nullptr;
    posix_memalign( &alignedPtr, alignBitCount, memSize );
    if( !alignedPtr )
    {
        LogMsg( LOG_FATAL, "Failed to align memory, insufficient memory available" );
    }

    return alignedPtr;

#else
    void* alignedPtr = nullptr;
    int res = posix_memalign( &alignedPtr, alignBitCount, memSize );
    if( res == EINVAL )
    {
        LogMsg( LOG_FATAL, "Failed to align memory, alignment is not a multiple of 2" );
    }
    else if( res == ENOMEM )
    {
        LogMsg( LOG_FATAL, "Failed to align memory, insufficient memory available" );
    }

    return alignedPtr;

#endif // defined(TARGET_OS_WINDOWS)
}

//============================================================================
void VxAlignedFree( void* alignMemPtr, bool glAllocated )
{
    if (!alignMemPtr)
    {
        return;
    }

#if defined(TARGET_OS_WINDOWS)
    return _aligned_free( alignMemPtr );

#elif defined(TARGET_OS_ANDROID)
    if( glAllocated )
    {
        free(alignMemPtr);
    }
    else
    {
        char *pFull = *(char **)(((char *)alignMemPtr) - sizeof(char *));
        free(pFull);
    }

#elif defined(TARGET_OS_LINUX)
    if( glAllocated )
    {
        free(alignMemPtr);
    }
    else
    {
        char *pFull = *(char **)(((char *)alignMemPtr) - sizeof(char *));
        free(pFull);
    }

#else
    free(*((void **)alignMemPtr - 1));

#endif // defined(TARGET_OS_WINDOWS)
}
