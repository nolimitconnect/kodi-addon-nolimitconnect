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
#include <CoreLib/config_corelib.h>

class VxBuffer
{
public:
	VxBuffer( int iPreAllocSize = 512, int iReallocSize = 1024 );
	virtual ~VxBuffer();

	int			                length( );
    int			                freeSpace( );
    void			            resetUsedLen( ) { m_iBufUsedLen = 0; }

	char *		                getDataPtr( void );

	void		                addData( char * pData, int iLen, bool nullTerminate = true );
	int			                readData( char * pData, int iLenToRead, int iReadOffs = 0, bool nullTerminate = true );
	int			                readAndRemoveData( char * pData, int iLenToRead, int iReadOffs = 0, bool nullTerminate = true );
	int			                removeData( int iLen, int iOffs = 0 );

	char *		                getWriteBuffer( int iBufferLen );
	void		                bytesWritten( int iBytesWritten, bool nullTerminate = true );
	
	void		                clear();
	void		                resize( int iAllocLenRequired );

private:
    char *				        m_pBuf;
    int					        m_iBufUsedLen;
    int					        m_iBufAllocTotalLen;
    int					        m_iBufAllocBlockLen;
};

