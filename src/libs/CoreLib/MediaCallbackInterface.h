#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxDefs.h"
#include "AudioCallbackSpaceAvailable.h"

#include <memory>
#include <vector>

class CamJpgVideo;
class PktVoiceReq;
class PktVideoFeedPic;
class PktVideoFeedPicChunk;
class VxGUID;

class MediaCallbackInterface : public AudioCallbackSpaceAvailable
{
public:
	virtual void				callbackPcm( VxGUID& feedId, int16_t* pcmData, uint16_t pcmDataLen ){};
	virtual void				callbackOpusEncoded( uint8_t* encodedAudio, uint16_t opusLenBytes ){};
	virtual void				callbackOpusPkt( PktVoiceReq* pktOpusAudio ){};

	virtual void				callbackVideoJpg( VxGUID& vidFeedId, std::shared_ptr<CamJpgVideo>& jpgVideo ){};
	virtual void				callbackVideoPktPic( VxGUID& onlineId, PktVideoFeedPic* pktVid, int pktsInSequence, int thisPktNum ){};
	virtual void				callbackVideoPktPicChunk( VxGUID& onlineId, PktVideoFeedPicChunk* pktVid, int pktsInSequence, int thisPktNum ){};
};
