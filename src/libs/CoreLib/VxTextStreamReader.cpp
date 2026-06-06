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

#include "VxTextStreamReader.h"
namespace
{
	const char LF = '\n';
	const char CR = '\r';
}

//============================================================================
VxTextStreamReader::VxTextStreamReader()
: m_iReadIdx(0)
{

}

//============================================================================
void VxTextStreamReader::setStreamData( const char* pData )
{
	m_strData = pData;
	m_iReadIdx = 0;
}

//============================================================================
const char* VxTextStreamReader::getStreamData( void )
{
	return m_strData.c_str();
}

//============================================================================
int VxTextStreamReader::skipStreamData( int iSkipLen )
{
	m_iReadIdx += iSkipLen;
	return iSkipLen;
}


bool VxTextStreamReader::readStreamLine( std::string& strRetLine )
{
	char readChar = 0;
	strRetLine = "";

	// Read line until CR & LF
	int	readLen = readStreamData( strRetLine, 1 );
	if( 0 >= readLen )
	{
		return false;
	}

	while( 0 < readLen ) 
	{
		int lastPos = strRetLine.size() - 1;
		readChar = strRetLine.at( lastPos );
		if (readChar == CR || readChar == LF) 
		{
			strRetLine = strRetLine.substr(0, lastPos);
			break;
		}
		readLen = readStreamData( strRetLine, 1 );
	}

	if (readChar == CR) 
	{
		std::string skipChar;
		readLen = readStreamData( skipChar, 1 );
		if( 0 < readLen ) 
		{
			readChar = skipChar.at(0);
			if (readChar != LF)
			{
				unwindStreamReadPosition(1);
			}
		}
	}

	return ( 0 < strRetLine.length() ) ? true : false;
}

//============================================================================
int VxTextStreamReader::readStreamData( std::string& strRetData, int iLenToRead )
{
	int iBufLen = m_strData.length() - m_iReadIdx;
	if( iBufLen <= 0 )
	{
		return 0;
	}
	int iCopyLen = ( iLenToRead < iBufLen ) ? iLenToRead : iBufLen;
	strRetData.append( m_strData, m_iReadIdx, iCopyLen );
	m_iReadIdx += iCopyLen;
	return iCopyLen;
}

//============================================================================
void VxTextStreamReader::resetStreamReadPosition( void )
{
	m_iReadIdx = 0;
}

//============================================================================
void VxTextStreamReader::unwindStreamReadPosition( int iLen )
{
	if( iLen <= m_iReadIdx )
	{
		m_iReadIdx -= iLen;
	}
	else
	{
		// TODO Error handling
	}
}

