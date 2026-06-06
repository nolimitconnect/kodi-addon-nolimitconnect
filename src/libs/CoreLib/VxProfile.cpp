//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "config_corelib.h"

#include "VirtFileMgr.h"
#include "VxProfile.h"
#include "VxFileUtil.h"
#include "VxParse.h"
#include "VxDebug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef TARGET_OS_WINDOWS
	#include <ctype.h> 
	#include <unistd.h> 
#endif // TARGET_OS_WINDOWS

#ifndef PROFILE_FAILED_CREATE_FILE
	#define PROFILE_FAILED_CREATE_FILE -1
	#define PROFILE_FAILED_WRITE_FILE -2
#endif //PROFILE_FAILED_CREATE_FILE

#define     MAX_INI_LINE_LENGTH 180

#ifdef TARGET_OS_WINDOWS
    #pragma warning( disable : 4996 )
#endif // TARGET_OS_WINDOWS

//============================================================================
// functions used only in this file
//============================================================================
static void MyCopyString( char *pDest, const char*pSrc, unsigned long ulMaxLen );

//============================================================================
/// write a binary value to an entry in a ini file
int32_t VxProfile::setProfileLong(	const char* pFileName, // name of file to access
									const char* pSection,	// which section to write to
									const char*pKey,		// which key to write
									const long lValue,		// value to write
									EnumRadix eRadix )		// BASE10=write as decimal, BASE16=write as hexadecimal
{
	char acBuf[ 80 ];
    if(BASE10 == eRadix)
    {
        sprintf(acBuf,"%d", (int)lValue);
    }
    else //base 16
    {
        sprintf(acBuf,"%x",(unsigned int)lValue);
    }

	return setProfileString( pFileName, pSection, pKey, acBuf );
}

//============================================================================
/// read a binary value from an entry in a ini file
long VxProfile::getProfileLong(	const char*pFileName,		// name of file to access
								const char*pSection,		// which section to read from
								const char*pKey,			// the key in the specified section
								long lDef,					// default value to return if error or entry doesn't exist
								EnumRadix eRadix )			// BASE10=read as decimal, BASE16=read as hexadecimal
{
	VFile* pgFile{ nullptr };
    char acBuf[MAX_INI_LINE_LENGTH];
    char *pEntryPos;
    char *pEnd;
    char acSection[MAX_INI_LINE_LENGTH];
    char acValue[32];
    size_t iLen, i;
	int iResult;

    if( 0 == ( pgFile = VFileOpen( pFileName, "r" ) ) )
	{
        return lDef;
	}
    iLen= strlen(pKey);
    sprintf(acSection,"[%s]",pSection);
    do
	{
        if( VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) )
		{
            VFileClose(pgFile);
            return lDef;
        }
    }while(strcmp(acBuf,acSection) );
    do
	{
		iResult = VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf ));
        if( iResult || (acBuf[0] == '\0') )
		{
			if( EOF == iResult )
				break;
			//if person didnt put cr at end of line readLine will return 1 even though acBuf has info
			size_t iLen = strlen( acBuf );
			if( iLen )
			{
				if( 0 == strncmp( acBuf, pKey, iLen )) //one last chance
	 				break;
				VFileClose(pgFile);
				return lDef;
			}
			else
			{

				continue;
			}
        }
    }while( strncmp( acBuf, pKey, iLen ) );
    pEntryPos= strrchr(acBuf,'=');
	if( pEntryPos )
	{
		pEntryPos++;  //increment past '='
        if( NULL == (pEntryPos = TrimLeadingSpaces(pEntryPos)))
        {
            return lDef;
        }
		if( strlen(pEntryPos) == 0 )
		{
			return lDef;
		}
		if( eRadix == BASE16 )
		{
			for( i=0; ( isxdigit(pEntryPos[i]) || tolower(pEntryPos[i] ) == 'x' ) && i< sizeof( acValue ); i++ )
			{
				acValue[i]=pEntryPos[i];
			}
		}
		else
		{
			for( i=0; (('-' == pEntryPos[i]) || isdigit(pEntryPos[i])) && i < sizeof( acValue ); i++ )
				acValue[i]=pEntryPos[i];
		}
		acValue[i]= '\0';
		VFileClose(pgFile);
		return( (long)strtol( acValue, &pEnd, eRadix ) );
	}
	else
	{
         VFileClose(pgFile);
	     return lDef;
	}
}

//============================================================================
/// write an floating decimal point value to a entry in a ini file
int32_t VxProfile::setProfileDouble(	const char*	pFileName,	// name of file to access
									const char*	pSection,	// which section to write to
									const char*	pKey,		// which key to write
									double			dValue )	// value to write
{
	char acBuf[ 80 ];
	sprintf( acBuf, "%f", dValue );
	return setProfileString( pFileName, pSection, pKey, acBuf );
}

//============================================================================
/// read a floating decimal point value from an entry in a ini file
double VxProfile::getProfileDouble(	const char*pFileName,	// name of file to access
									const char*pSection,	// which section to read from
									const char*pKey, 		// which key to read
									double dDef )			// default value to return if error or entry doesn't exist
{
    VFile *pgFile;
    char acBuf[MAX_INI_LINE_LENGTH];
    char *pEntryPos;
    char acSection[MAX_INI_LINE_LENGTH];
    size_t iLen;
	int iResult;

    if( 0 == ( pgFile = VxFileUtil::fileOpen( pFileName, "r" ) ) )
	{
        return dDef;
	}

    iLen= strlen(pKey);
    sprintf(acSection,"[%s]",pSection);
    do
	{
        if( VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) )
		{
            VFileClose(pgFile);
            return dDef;
        }
    }while(strcmp(acBuf,acSection) );

    do
	{
        iResult = VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf ));
        if( ( 0 != iResult) || (acBuf[0] == '\0') )
		{
			if( EOF == iResult )
				break;
			//if person didnt put cr at end of line readLine will return 1 even though acBuf has info
			if( 0 == strncmp( acBuf, pKey, iLen )) //one last chance
				break;
            VFileClose(pgFile);
            return dDef;
        }
    }while( strncmp( acBuf, pKey, iLen ) );

    pEntryPos = strrchr(acBuf,'=');
    pEntryPos++;
    if( NULL == (pEntryPos = TrimLeadingSpaces(pEntryPos)))
    {
        return dDef;
    }

    VFileClose(pgFile);
    return atof( pEntryPos );
}

//============================================================================
/// read a string from a entry in a ini file
int VxProfile::getProfileString( const char*pFileName,		// name of file to access
						const char*pSection,		// which section to read from
						const char*pKey,			// which key to read
						const char*pDef,			// default value to return if error or entry doesn't exist
						char *pRetBuf,				// buffer to write string into
						int iRetBufLen )			// maximum bytes that can be written into the buffer
{
    VFile *pgFile;
    char acBuf[ MAX_INI_LINE_LENGTH ];
    char acSection[ MAX_INI_LINE_LENGTH ];
    char * pEntryPos;
    size_t iLen;
	int iResult;

    if( (pgFile = VxFileUtil::fileOpen(pFileName,"r"))== NULL )
	{
		LogMsg( LOG_ERROR, "getProfileString: could not open file %s\n", pFileName );
		MyCopyString( pRetBuf, pDef, iRetBufLen );
        return 0;
	}

    iLen= strlen(pKey);
    sprintf( acSection,"[%s]",pSection );
    do
	{
        if( VxFileUtil::readLine( pgFile, acBuf, sizeof( acBuf ) ) )
		{
			//LogMsg( LOG_ERROR, "getProfileString: could  find section %s\n", acSection );
            VFileClose(pgFile);
            MyCopyString( pRetBuf, pDef,iRetBufLen-1);
            return (int)strlen( pRetBuf );
        }
    }while(strcmp(acBuf,acSection) );

    do
	{
        iResult = VxFileUtil::readLine( pgFile, acBuf, sizeof( acBuf ));
        if( ( 0 != iResult ) || ( acBuf[0] == '\0' ) )
		{
			if( EOF == iResult )
			{
				//LogMsg( LOG_ERROR, "getProfileString: could not find key %s\n", pKey );
				break;
			}

 			//if person didnt put cr at end of line readLine will return 1 even though acBuf has info
			if( 0 == strncmp( acBuf, pKey, iLen )) //one last chance
			{
				//LogMsg( LOG_ERROR, "getProfileString: could not find key %s\n", pKey );
				break;
			}
			LogMsg( LOG_ERROR, "getProfileString: could not find key %s\n", pKey );
            VFileClose(pgFile);
            MyCopyString( pRetBuf, pDef, iRetBufLen-1 );
            return (int)strlen( pRetBuf ) ;
        }
    }while( strncmp( acBuf, pKey, iLen) );

    pEntryPos = strchr(acBuf,'=');
	if( pEntryPos )
	{
		pEntryPos++;
		MyCopyString( pRetBuf, pEntryPos, iRetBufLen-1 );
	}
	else
	{
		pRetBuf[0]=0;
	}

    VFileClose(pgFile);
    return (int)strlen(pRetBuf);
}

//============================================================================
/// read a string from a entry in a ini file
int VxProfile::getProfileString( const char*pFileName,		// name of file to access
						const char*pSection,		// which section to read from
						const char*pKey,			// which key to read
						const char*pDef,			// default value to return if error or entry doesn't exist
						std::string &	csRetValue )	// value read from ini
{
	char as8Buf[ VX_MAX_PATH * 2 ];
	as8Buf[ 0 ] = 0;
	int iResult = getProfileString( pFileName,		// name of file to access
									pSection,		// which section to read from
									pKey,			// which key to read
									pDef,			// default value to return if error or entry doesn't exist
									as8Buf,
									VX_MAX_PATH * 2 );	// value read from ini
	csRetValue = as8Buf;
	return iResult;

}

//============================================================================
/// write a string from a entry in a ini file
int32_t VxProfile::setProfileString( const char*pFileName,	// name of file to access
							const char*pSection,	// which section to write to
							const char*pKey,		// which key to write
							const char*pStr, ... )	// string to write ( can be formatted like printf )
{
    VFile *pgFile;
	VFile *pgWrFile;
    char acTempFileName[VX_MAX_PATH];
    char acBuf[MAX_INI_LINE_LENGTH];
    char acSection[MAX_INI_LINE_LENGTH];
	char acString[ MAX_INI_LINE_LENGTH ];
    size_t iLen;

	// format string
	va_list arg_ptr;
	va_start(arg_ptr, pStr );
	vsnprintf( acString, MAX_INI_LINE_LENGTH, pStr, arg_ptr );
	va_end(arg_ptr);

    iLen = strlen( pKey );
#ifdef TARGET_OS_WINDOWS
    tmpnam( acTempFileName );
#else
    mkstemp( acTempFileName );
#endif // TARGET_OS_WINDOWS
    sprintf( acSection,"[%s]",pSection );

    //=== Open file to read. Otherwise create one ===//
    if( 0 == (pgFile = VxFileUtil::fileOpen( pFileName, "r" )) )
	{
        if(  0 == (pgFile = VxFileUtil::fileOpen( pFileName,"w+")) )
		{
			LogMsg( LOG_ERROR, "setProfileString: could create file %s\n", pFileName );
			return PROFILE_FAILED_CREATE_FILE;
		}
        VFilePrintf(pgFile,"%s\n",acSection );
        VFilePrintf(pgFile,"%s=%s\n",pKey, acString );
        VFileClose(pgFile);
        return 0;
    }

    //=== Open file to write modified profile ===//
    if( 0 == ( pgWrFile = VFileOpen( acTempFileName,"w")) )
	{
		LogMsg( LOG_ERROR, "setProfileString: could create file %s\n", acTempFileName );
        VFileClose(pgFile);
        return PROFILE_FAILED_WRITE_FILE;
    }
	//LogMsg( DRAC_UTIL_STATUS, "setProfileString: created temp file %s\n", acTempFileName );

    //=== Read file lines until a Section is matched or EOF ===//
    do
	{
        if( VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) )
		{
            //=== Failed to find pSection so add one to the end of file ===//
            VFilePrintf(pgWrFile,"\n%s\n",acSection );
            VFilePrintf(pgWrFile,"%s=%s\n",pKey,acString );

            //=== rename file and clean up ==//
            VFileClose( pgFile );
            VFileClose( pgWrFile );
            unlink( pFileName );
            rename( acTempFileName, pFileName );
            return 0;
        }
        VFilePrintf(pgWrFile,"%s\n",acBuf);
    }while( strcmp( acBuf,acSection ) );

	//=== Section was found ===//
	// copy to temporary write file and add the Entry if Entry not found
    while( true )
	{
        if( VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) )
		{   //entry wasnt found
            VFilePrintf(pgWrFile,"%s=%s\n",pKey, acString );  // add entry to current Section
            VFileClose( pgFile );
            VFileClose( pgWrFile );
            unlink( pFileName );
            rename( acTempFileName, pFileName );
            return 0;
        }

        if( (!strncmp(acBuf,pKey,iLen)) || (acBuf[0] == '\0') ||   (acBuf[0] == '[') )
		{
			if( acBuf[0] == '[' )
			{
				// reached end of section without finding our entry .. so write it now
		        VFilePrintf(pgWrFile,"%s=%s\n",pKey,acString );
                VFilePrintf(pgWrFile,"%s\n",acBuf );
				// write out the rest of the ini file
				while( !VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) )
				{
                    VFilePrintf(pgWrFile,"%s\n",acBuf );
				}
				goto done;
			}
            break;
		}

        VFilePrintf(pgWrFile,"%s\n",acBuf);
    }

    //=== Write Entry data under Section or Append to the end ===//
    if( acBuf[0] == '\0' )
	{
		//
        VFilePrintf(pgWrFile,"%s=%s\n",pKey,acString );
        do
		{
            VFilePrintf(pgWrFile,"%s\n",acBuf);
        }while( !VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) );
    }
	else
	{
        VFilePrintf(pgWrFile,"%s=%s\n",pKey,acString );
        while( !VxFileUtil::readLine(pgFile,acBuf, sizeof( acBuf )) )
		{
            VFilePrintf(pgWrFile,"%s\n",acBuf );
		}
    }

done:
    //=== Rename file and clean up ===//
    VFileClose(pgWrFile);
    VFileClose(pgFile);
    unlink(pFileName);
#ifdef TARGET_OS_WINDOWS
	if( 0 != rename(acTempFileName,pFileName) )
	{
		LogMsg( LOG_ERROR, "VxProfile rename failed: %s\n", strerror( errno ) );
	}
#else //LINUX
	// linux will not rename if destination and source devices are different so copy instead
	char as8Buf[ VX_MAX_PATH * 2 ];
	sprintf( as8Buf, "cp %s %s\n", acTempFileName, pFileName );
	system( as8Buf );
	unlink( acTempFileName );
#endif // LINUX
    return 0;
}

//============================================================================
void MyCopyString(char *pDest, const char*pSrc, unsigned long ulMaxLen)
{
	if(! pSrc && pDest)
	{
		pDest[0] = 0;
		return;
	}

	if(! pSrc && ! pDest)
	{
		return;
	}

    if(pDest)
    {
        if( strlen( pSrc ) >= ulMaxLen )
        {
            strncpy( pDest, pSrc, ulMaxLen );
            pDest[ ulMaxLen ] = 0;
        }
        else
		{
            strcpy( pDest, pSrc );
		}
    }
}








