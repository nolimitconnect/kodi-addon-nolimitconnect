//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "MJPEGWriter.h"
#include "SndDefs.h"

#include <P2PEngine/P2PEngine.h>
#include <MediaProcessor/MediaProcessor.h>

#include <PktLib/PktVoiceReq.h>
#include <PktLib/PktVoiceReply.h>
#include <PktLib/PktChatReq.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VirtFileMgr.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxMacros.h>
#include <CoreLib/VxFileShredder.h>

namespace
{
	const int MS_PER_AUDIO_CHUNK = AUDIO_MS_PER_FRAME;
}

//============================================================================
MJPEGWriter::MJPEGWriter( P2PEngine& engine, MediaProcessor& mediaProcessor )
: m_Engine( engine )
, m_MediaProcessor( mediaProcessor )
, m_FeedId()
, m_EPluginType( ePluginTypeMJPEGWriter )
, m_IsRecording( false )
, m_IsRecordingPaused( false )
, m_FileName( "" )
, m_FileHandle( 0 )
, m_MicroSecBetweenFrames( 0 )
, m_TotalJpgDataLen( 0 )
, m_TotalFrameCnt( 0 )
, m_TotalElapsedMs( 0 )
, m_IsFirstFrameAfterResumeRecording( 0 )
, m_PrevFrameJpgLen( 0 )
, m_ImageWidth( 320 )
, m_ImageHeight( 240 )
{
	m_MediaSessionId.initializeWithNewVxGUID();
}

//============================================================================
bool MJPEGWriter::fromGuiVideoRecord( EVideoRecordState eRecState, VxGUID& feedId, const char* fileName  )
{
	bool result = false;
	switch( eRecState )
	{
	case eVideoRecordStateStopRecording:
		if( getIsRecording() )
		{
			stopAviWrite();
			result = true;
		}

		break;

	case eVideoRecordStateCancelRecording:
		if( getIsRecording() )
		{
			stopAviWrite( true );
			result = true;
		}

		break;

	case eVideoRecordStateStartRecording:
		if( !getIsRecording() )
		{
			// windows does 20fps..
			m_FeedId = feedId;
			result = startAviWrite( fileName, 50000, false );
		}

		break;

	case eVideoRecordStateStartRecordingInPausedState:
		if( !getIsRecording() )
		{
			// windows does 20fps..
			result = startAviWrite( fileName, 50000, true );
		}

		break;

	case eVideoRecordStatePauseRecording:
		setIsRecordingPaused( true );
		break;

	case eVideoRecordStateResumeRecording:
		setIsRecordingPaused( false );
		break;

	case eVideoRecordStateDisabled:
	case eVideoRecordStateError:
	default:
		break;
	}

	return result;
}

//============================================================================
bool MJPEGWriter::startAviWrite( const char* fileName, uint32_t timeBetweenFramesMicroSec, bool beginInPausedState )
{
	stopAviWrite();
	m_FileName = fileName;
	m_FileHandle = VFileOpen( m_FileName.c_str(), "wb+" );
	if( 0 == m_FileHandle )
	{
		LogMsg( LOG_ERROR, "MJPEGWriter::startAviWrite could not open file %s", m_FileName.c_str() );
		return false;
	}

	m_MicroSecBetweenFrames				= timeBetweenFramesMicroSec;
	m_FrameOffsetList.clear();
	m_TotalJpgDataLen					= 0;
	m_TotalFrameCnt						= 0;
	m_TotalElapsedMs					= 0;
	m_IsFirstFrameAfterResumeRecording	= true;

	// start with a guesstimate
	setAviParameters( 320, 240, m_MicroSecBetweenFrames, 1000, 1000 * 8000 );

	if( false == writeRiffHeader() )
	{
		VFileClose( m_FileHandle );
		return false;
	}

	if( false == writeVideoHeader() )
	{
		VFileClose( m_FileHandle );
		return false;
	}

	setIsRecordingPaused( beginInPausedState );
	setIsRecording( true );
    m_Engine.getMediaProcessor().wantMediaInput( m_Engine.getMyOnlineId(), eMediaInputVideoJpg, this, eMediaModuleMediaWriter, m_MediaSessionId, true );
    m_Engine.getMediaProcessor().wantMediaInput( m_Engine.getMyOnlineId(), eMediaInputAudioPcm, this, eMediaModuleMediaWriter, m_MediaSessionId, true );
	return true;
}

//============================================================================
void MJPEGWriter::stopAviWrite( bool deleteFile )
{
	if( getIsRecording() )
	{
		setIsRecordingPaused( true );
        m_Engine.getMediaProcessor().wantMediaInput( m_Engine.getMyOnlineId(), eMediaInputAudioPcm, this, eMediaModuleMediaWriter, m_MediaSessionId, false );
        m_Engine.getMediaProcessor().wantMediaInput( m_Engine.getMyOnlineId(), eMediaInputVideoJpg, this, eMediaModuleMediaWriter, m_MediaSessionId, false );

		if( deleteFile )
		{
			// no need to finish updating file.. just close it and delete it		
			setIsRecording( false );
			VFileClose( m_FileHandle );
			m_FileHandle = nullptr;
			LogMsg( LOG_ERROR, "MJPEGWriter::stopAviWrite shredding file %s", m_FileName.c_str() );
			GetVxFileShredder().shredFile( m_FileName );
			return;
		}

		// save totals before changed
		m_TotalFrameCnt			    = (uint32_t)m_FrameOffsetList.size();
		uint32_t audioTotalChunks	= ( uint32_t )m_PcmOffsetList.size();
		uint32_t videoTotalFrames	= m_TotalFrameCnt - audioTotalChunks;

		uint32_t lastOffsetWritten	= 0;
		uint32_t lastLengthWritten	= 0;

		if( m_FrameOffsetList.size() > 0 )
		{
			AviFourCC idxFourCC( 'i', 'd', 'x', '1' );
			if( sizeof( AviFourCC ) != VFileWrite( &idxFourCC, 1, sizeof( AviFourCC ), m_FileHandle ) )
			{
				LogMsg( LOG_ERROR, "MJPEGWriter::stopAviWrite error %d writing file %s", VxGetLastError(), m_FileName.c_str() );
				closeAviFile();
				return;
			}

			uint32_t idxListLen = ( uint32_t )(16 * m_FrameOffsetList.size());
			if( sizeof( idxListLen ) != VFileWrite( &idxListLen, 1, sizeof( idxListLen ), m_FileHandle ) )
			{
				LogMsg( LOG_ERROR, "MJPEGWriter::stopAviWrite error %d writing file %s", VxGetLastError(), m_FileName.c_str() );
				closeAviFile();
				return;
			}

			m_TotalFrameCnt = ( uint32_t )m_FrameOffsetList.size();
			uint32_t lastDataOffset = m_FrameOffsetList.size() ? m_FrameOffsetList[ m_FrameOffsetList.size() - 1 ] + m_PrevFrameJpgLen + 8 : 4;
			m_FrameOffsetList.push_back( lastDataOffset );

			AviFrameIndex frameIdx;
			std::vector<uint32_t>::iterator iter;
			std::vector<uint32_t>::iterator audIter;
			uint32_t prevOffs = 4;
			uint32_t nextOffs;
			uint32_t dataLen;
			// don't count the first one... it was special with fixed value 4
			m_FrameOffsetList.erase( m_FrameOffsetList.begin() );
			int idxCnt = 0;
			for( iter = m_FrameOffsetList.begin(); iter != m_FrameOffsetList.end(); ++iter )
			{
				nextOffs = *iter;
				bool isAudioOffset = false;
				for( audIter = m_PcmOffsetList.begin(); audIter != m_PcmOffsetList.end(); ++audIter )
				{
					if( *audIter == prevOffs )
					{
						isAudioOffset = true;
						m_PcmOffsetList.erase( audIter );
						break;
					}
				}

				if( isAudioOffset )
				{
					frameIdx.m_00db.setFourCC( '0', '1', 'w', 'b' );
				}
				else
				{
					frameIdx.m_00db.setFourCC( '0', '0', 'd', 'c' );
				}

				dataLen = nextOffs - ( prevOffs + 8 );
				frameIdx.setOffset( (uint32_t)prevOffs );
				frameIdx.setDataLen( (uint32_t)dataLen );
				lastOffsetWritten = (uint32_t)prevOffs;
				lastLengthWritten = (uint32_t)dataLen;

				idxCnt++;
				//LogMsg( LOG_INFO, "Avi Index %d len %d offset %d\n", 
				//	(uint32_t)idxCnt,
				//	(uint32_t)dataLen,
				//	(uint32_t)prevOffs
				// );

				prevOffs = nextOffs;
				if( sizeof( AviFrameIndex ) != VFileWrite( &frameIdx, 1, sizeof( AviFrameIndex ), m_FileHandle ) )
				{
					LogMsg( LOG_ERROR, "MJPEGWriter::stopAviWrite error %d writing file %s", VxGetLastError(), m_FileName.c_str() );
					closeAviFile();
					return;
				}
			}
		}

		closeAviFile();
		// rewrite the header with the correct info
		// use the total ms of audio because it will be the most accurate
		double totalAudioTime = audioTotalChunks * MS_PER_AUDIO_CHUNK;
		double msPerFrame = totalAudioTime / videoTotalFrames;
		if( msPerFrame < 32 )
		{
			msPerFrame = 32; // couldn't be faster than 30 fps
		}

		if( msPerFrame > 200 )
		{
			msPerFrame = 200; // couldn't be slower than 5 fps
		}

		setAviParameters(	320, 
							240, 
							(uint32_t)(msPerFrame * 1000),
							videoTotalFrames, 
							(uint32_t)m_TotalJpgDataLen );

		m_AviHdr.m_AudioStream.setPcmAudioRate( AUDIO_DEVICE_SAMPLE_RATE );
		m_AviHdr.m_AudioStream.setPcmChunkTotals( audioTotalChunks, AUDIO_BUF_SIZE ); 

		uint64_t fileLen = VxFileUtil::getFileLen( m_FileName.c_str() );
		m_RiffHdr.setTotalRiffLen( (uint32_t)(fileLen - 8) );
		m_AviHdr.m_FramesBeginHdr.m_SizeOfOwnerStructMinus8 = lastLengthWritten + lastOffsetWritten + 8;
		m_FileHandle = VxFileUtil::fileOpen( m_FileName.c_str(), "rb+" );
		if( 0 == m_FileHandle )
		{
			LogMsg( LOG_ERROR, "MJPEGWriter::rewrite headers could not open file %s", m_FileName.c_str() );
		}
		else if( false == writeRiffHeader() )
		{
			LogMsg( LOG_ERROR, "MJPEGWriter::rewrite riff hdr fail file %s", m_FileName.c_str() );
		}
		else if( false == writeVideoHeader() )
		{
			LogMsg( LOG_ERROR, "MJPEGWriter::rewrite video hdr fail file %s", m_FileName.c_str() );
		}

		VFileClose( m_FileHandle );
		setIsRecording( false );
	}
}

//============================================================================
void MJPEGWriter::setIsRecordingPaused( bool pause )
{
	if( pause )
	{
		if( getIsRecording() )
		{
			if( !m_IsFirstFrameAfterResumeRecording )
			{
				m_TotalElapsedMs += m_RecordElapseTimer.elapsedMs();
			}
		}
	}
	else
	{
		m_IsFirstFrameAfterResumeRecording = true;
		m_RecordElapseTimer.startTimer(); // restart timer because of time while we were paused
	}

	m_IsRecordingPaused = pause;
}

//============================================================================
bool MJPEGWriter::setAviParameters(	 uint32_t	imageWidth, 
									 uint32_t	imageHeight, 
									 uint32_t	microSecBetweenFrames, 
									 uint32_t	frameCnt, 
									 uint32_t	totalJpgDataLen )
{
	m_AviHdr.setAviParameters(	imageWidth,
								imageHeight,
								microSecBetweenFrames,
								frameCnt,
								totalJpgDataLen );
	return true;
}

//============================================================================
bool MJPEGWriter::writeRiffHeader( void )
{
	if( sizeof( AviRiffHeader ) != VFileWrite( &m_RiffHdr, 1, sizeof( AviRiffHeader ), m_FileHandle ) )
	{
		LogMsg( LOG_ERROR, "MJPEGWriter::writeRiffHeader error %d writing file %s", VxGetLastError(), m_FileName.c_str() );
		return false;
	}

	return true;
}

//============================================================================
bool MJPEGWriter::writeVideoHeader( void )
{
	if( sizeof( AviVideoHdr ) != VFileWrite( &m_AviHdr, 1, sizeof( AviVideoHdr ), m_FileHandle ) )
	{
		LogMsg( LOG_ERROR, "MJPEGWriter::writeVideoHeader error %d writing file %s", VxGetLastError(), m_FileName.c_str() );
		return false;
	}

	return true;
}

//============================================================================
void MJPEGWriter::callbackVideoJpg( VxGUID& vidFeedId, std::shared_ptr<CamJpgVideo>& jpgVideo )
{
	if( !getIsRecording() || getIsRecordingPaused() || ( vidFeedId != m_FeedId ) )
	{
		return;
	}

	uint8_t* pu8Jpg = jpgVideo->m_VidData.get();
	uint32_t u32JpgDataLen = jpgVideo->m_VidDataLen;

	uint32_t lenToWrite = ROUND_TO_4BYTE_BOUNDRY( u32JpgDataLen );
	if( 0 == lenToWrite )
	{
		return;
	}

	if( m_IsFirstFrameAfterResumeRecording )
	{
		m_IsFirstFrameAfterResumeRecording = false;
		// start elapse time timer
		m_RecordElapseTimer.startTimer();
	}

	// audio and video come in on different threads.. need mutex access
	m_AviFileAccessMutex.lock();
	if( 0 == m_FileHandle )
	{
		m_AviFileAccessMutex.unlock();
		return;
	}

	uint32_t dataOffset = m_FrameOffsetList.size() ? m_FrameOffsetList[ m_FrameOffsetList.size() - 1 ] + m_PrevFrameJpgLen + 8 : 4;
	m_FrameOffsetList.push_back( dataOffset );
	m_PrevFrameJpgLen = lenToWrite;
	m_TotalJpgDataLen += lenToWrite;
	m_AviJpgHdr.m_DataLen = lenToWrite;
	//LogMsg( LOG_INFO, "Avi Rec Jpg Frame %d len %d offset %d\n",	m_FrameOffsetList.size(),
	//																m_PrevFrameJpgLen,
	//																dataOffset );
	//char dumpFileName[256];
	//sprintf( dumpFileName, "F:/%d.jpg", m_FrameOffsetList.size() );
	//VxFileUtil::writeWholeFile( dumpFileName, pu8Jpg, u32JpgDataLen );

	// we replace the JIFF after the first 6 bytes in jpg with AVI1
	memcpy( m_AviJpgHdr.m_First6BytesOfJpg, pu8Jpg, 6 );

	if( sizeof( AviJpgHdr ) != VFileWrite( &m_AviJpgHdr, 1, sizeof( AviJpgHdr ), m_FileHandle ) )
	{
		m_AviFileAccessMutex.unlock();
		LogMsg( LOG_ERROR, "MJPEGWriter::fromGuiJpgFrame error %d writing frame hdr to file %s", VxGetLastError(), m_FileName.c_str() );
		return;
	}

	if( lenToWrite - 10 != VFileWrite( &pu8Jpg[10], 1, lenToWrite-10, m_FileHandle ) )
	{
		m_AviFileAccessMutex.unlock();
		LogMsg( LOG_ERROR, "MJPEGWriter::fromGuiJpgFrame error %d writing jpg len %d to file %s", VxGetLastError(), lenToWrite, m_FileName.c_str() );
		return;
	}

	m_AviFileAccessMutex.unlock();
}

//============================================================================
void MJPEGWriter::callbackPcm( VxGUID& feedId, int16_t * pcmData, uint16_t pcmDataLen )
{
	if( !getIsRecording() || getIsRecordingPaused() || ( feedId != m_FeedId ) )
	{
		return;
	}

	if( 0 == m_FrameOffsetList.size() )
	{
		// wait until at least one video frame
		return;
	}

	//if( feedId != m_FeedId )
	//{
	//	return;
	//}

	uint32_t lenToWrite = ROUND_TO_4BYTE_BOUNDRY( pcmDataLen );
	if( 0 == lenToWrite )
	{
		return;
	}

	// audio and video come in on different threads.. need mutex access
	m_AviFileAccessMutex.lock();
	if( 0 == m_FileHandle )
	{
		m_AviFileAccessMutex.unlock();
		return;
	}

	uint32_t dataOffset = m_FrameOffsetList.size() ? m_FrameOffsetList[ m_FrameOffsetList.size() - 1 ] + m_PrevFrameJpgLen + 8 : 4;
	m_FrameOffsetList.push_back( dataOffset );
	m_PcmOffsetList.push_back( dataOffset );
	m_PrevFrameJpgLen = lenToWrite;
	m_TotalJpgDataLen += lenToWrite;
	m_AviAudioHdr.m_DataLen = (uint32_t)lenToWrite;
	//LogMsg( LOG_INFO, "Avi Rec Pcm Frame %d len %d offset %d\n", 
	//	m_FrameOffsetList.size(),
	//	(uint32_t)m_PrevFrameJpgLen,
	//	(uint32_t)dataOffset
	//	);
	if( sizeof( AviAudioHdr ) != VFileWrite( &m_AviAudioHdr, 1, sizeof( AviAudioHdr ), m_FileHandle ) )
	{
		m_AviFileAccessMutex.unlock();
		LogMsg( LOG_ERROR, "MJPEGWriter::fromGuiJpgFrame error %d writing frame hdr to file %s", VxGetLastError(), m_FileName.c_str() );
		return;
	}

	if( lenToWrite != VFileWrite( pcmData, 1, lenToWrite, m_FileHandle ) )
	{
		m_AviFileAccessMutex.unlock();
		LogMsg( LOG_ERROR, "MJPEGWriter::fromGuiJpgFrame error %d writing jpg len %d to file %s", VxGetLastError(), lenToWrite, m_FileName.c_str() );
		return;
	}

	m_AviFileAccessMutex.unlock();
}

//============================================================================
void MJPEGWriter::closeAviFile( void )
{
	m_AviFileAccessMutex.lock();
	if( 0 != m_FileHandle )
	{
		VFileClose( m_FileHandle );
		m_FileHandle = nullptr;
	}

	m_AviFileAccessMutex.unlock();
}

