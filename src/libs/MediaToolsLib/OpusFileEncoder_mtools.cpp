//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OpusFileEncoder.h"

#include "OggStream.h"

#include <opus/OpusCodec.h>

#include <CoreLib/IsBigEndianCpu.h>
#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>

#include <string.h>
#include <memory.h>

//============================================================================
OpusFileEncoder::OpusFileEncoder(  )
: m_OggStream( * ( new OggStream() ) )
, m_FileHandle( nullptr )
, m_TotalSndFramesInFile( 0 )
, m_EncoderInitialized( false )
{
}

//============================================================================
OpusFileEncoder::~OpusFileEncoder()
{
	delete m_OpusCodec;
	m_OpusCodec = nullptr;
	delete &m_OggStream;
}

//============================================================================
bool OpusFileEncoder::beginFileEncode( const char* fileName, int sampleRate, int channels )
{
	m_FileName = fileName;
	m_FileHandle = VFileOpen( fileName, "wb+" );
	if( 0 == m_FileHandle )
	{
		LogMsg( LOG_ERROR, "OpusFileEncoder::beginWrite could not open file to write %s", fileName );
		return false;
	}

	m_TotalSndFramesInFile = 0;
	m_EncoderInitialized = createAudioEncoder( sampleRate, channels );
	if( m_EncoderInitialized )
	{
		//Initialize Ogg stream struct
		m_EncoderInitialized = m_OggStream.openOggStream( m_FileHandle );
		if( m_EncoderInitialized )
		{
			//Write opus header into buffer
			unsigned char opusHeaderData[ 100 ];
			// writes "OpusHead" and channel/stream info into buffer ( 19 bytes )
			int pktDataLen = m_OggStream.writeHeader( m_OpusHeader, opusHeaderData, 100 ); 
			if( 0 == pktDataLen )
			{
				m_EncoderInitialized = false;
			}
		}
		else
		{
			VFileClose( m_FileHandle );
			m_FileHandle = 0;
			VxFileUtil::deleteFile( m_FileName.c_str() );
		}
	}
	else
	{
		VFileClose( m_FileHandle );
		m_FileHandle = 0;
		VxFileUtil::deleteFile( m_FileName.c_str() );
	}

	return m_EncoderInitialized;
}

//============================================================================
bool OpusFileEncoder::createAudioEncoder( int sampleRate, int channels )
{
	if( m_OpusCodec )
	{
		delete m_OpusCodec;
	}

	m_OpusCodec = new OpusCodec( sampleRate, channels );
	return m_OpusCodec != nullptr;
}


//============================================================================
int OpusFileEncoder::encodePcmData( int16_t* pcmData, uint16_t pcmDataLen )
{
	if( false == m_EncoderInitialized )
	{
		LogMsg( LOG_ERROR, "ERROR: OpusFileEncoder::writePcmData not initialized" );
		return false;
	}

	int result = 0;

	uint8_t encodedBuf[ AUDIO_BUF_SIZE ];
	std::vector<uint16_t> opusEncodedLenList;
	int encodedLen = m_OpusCodec->encode(	pcmData, 
												pcmDataLen, 
												encodedBuf,
												sizeof( encodedBuf ));
	if( encodedLen )
	{

		result = m_OggStream.writeEncodedFrame( encodedBuf, encodedLen );
		if( result )
		{
			m_TotalSndFramesInFile++;
		}
		else
		{
			LogMsg( LOG_ERROR, "ERROR: OpusFileEncoder::writePcmData failed to write encoded frame to file" );
		}

	}
	else
	{
		LogMsg( LOG_ERROR, "ERROR: OpusFileEncoder::encodePcmData failed to encode pcm data" );
	}

	return result;
}

//============================================================================
int OpusFileEncoder::writeEncodedFrame( uint8_t* encodedFrameData, int32_t encodedLen )
{
	if( false == m_EncoderInitialized )
	{
		LogMsg( LOG_ERROR, "ERROR: OpusFileEncoder::writeEncodedFrame not initialized" );
		return 0;
	}

	if( encodedLen <= 0 )
	{
		LogMsg( LOG_ERROR, "ERROR: OpusFileEncoder::writeEncodedFrame invalid encoded len %d", encodedLen );
		return 0;
	}

	int result = m_OggStream.writeEncodedFrame( encodedFrameData, encodedLen );
	if( result )
	{
		m_TotalSndFramesInFile++;
	}

	return result;
}

//============================================================================
void OpusFileEncoder::finishFileEncode( void )
{
	int64_t bytesWritten = m_OggStream.closeOggStream();
	// the file was closed by Ogg Stream
	m_FileHandle = nullptr;

	if( bytesWritten )
	{
		bool writeResult = false;
		uint64_t fileLen = VxFileUtil::getFileLen( m_FileName.c_str() );
		VFile* fileHandle = VFileOpen( m_FileName.c_str(), "rb+" );
		if( 0 != fileHandle )
		{
			writeResult = writeTotalSndFrames( fileHandle );
			VFileSeek64( fileHandle, fileLen );
			VFileClose( fileHandle );
		}

		if( false == writeResult )
		{
			LogMsg( LOG_ERROR, "OpusFileEncoder::finishFileEncode could not write frame count to file %s", m_FileName.c_str() );
		}
	}
}

//============================================================================
bool OpusFileEncoder::writeTotalSndFrames( VFile * fileHandle )
{
	bool writeSuccess = false;
	std::string hexTotal;
	VxFileUtil::u64ToHexAscii( htonU64( m_TotalSndFramesInFile ), hexTotal );

	const int hexStrLen = 16;
	if( hexStrLen != hexTotal.length() )
	{
		vx_assert( false );
	}

    uint32_t totalFramesOffs = 0xAD;
	if( ( hexStrLen == hexTotal.length() ) && ( 0  == VxFileUtil::fileSeek( fileHandle, totalFramesOffs ) ) )
	{
        char readBuf[ 512 ];
        if( 0 == VFileSeek64( fileHandle, NO_LIMIT_OPUS_SIGNITURE_OFFS ) )
        {
            if( sizeof( readBuf ) == VFileRead( readBuf, 1, sizeof( readBuf ), fileHandle ) )
            {
                for( int i = 0; i < 10; i++ )
                {
                    if( 0 == strncmp( NO_LIMIT_OPUS_SIGNITURE, &readBuf[ i ], NO_LIMIT_OPUS_SIGNITURE_LEN ) )
                    {
                        memcpy( &readBuf[ i + NO_LIMIT_OPUS_SIGNITURE_LEN ], hexTotal.c_str(), hexStrLen);

                        if( (0 == VFileSeek( fileHandle, NO_LIMIT_OPUS_SIGNITURE_OFFS, SEEK_SET )) &&
                            sizeof( readBuf ) == VFileWrite( readBuf, 1, sizeof( readBuf ), fileHandle) )
                        {
                            writeSuccess = true;
                            break;
                        }
                    }
                }
            }
        }
	}
	else
	{
		LogMsg( LOG_ERROR, "OpusFileEncoder::%s wrong total bytes in hex length %s", __func__, m_FileName.c_str() );
		vx_assert( false );
	}

	if( false == writeSuccess )
	{
		LogMsg( LOG_ERROR, "OpusFileEncoder::%s FAILED %s", __func__, m_FileName.c_str() );
	}

	return writeSuccess;
}



