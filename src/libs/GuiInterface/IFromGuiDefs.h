#pragma once
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "IDefs.h"

#include <CoreLib/VxKeyDefs.h>
#include <CoreLib/VxFileTypeMasks.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/AssetDefs.h>
#include <CoreLib/MediaCallbackInterface.h>

// Plugin server state
enum EPluginServerState
{
	ePluginServerStateDisabled,		//< server is disabled
	ePluginServerStateStarted,		//< server is running
	ePluginServerStateStopped,		//< server is enabled but not running

	eMaxPluginServerState
};

// Video recording state
enum EVideoRecordState
{
	eVideoRecordStateDisabled,
	eVideoRecordStateStopRecording,
	eVideoRecordStateStartRecording,
	eVideoRecordStateStartRecordingInPausedState,
	eVideoRecordStatePauseRecording,
	eVideoRecordStateResumeRecording,
	eVideoRecordStateCancelRecording,
	eVideoRecordStateError,

	eMaxVideoRecordState
};

//! \public Audio recording state
enum ESndRecordState
{
	eSndRecordStateDisabled,
	eSndRecordStateStopRecording,
	eSndRecordStateStartRecording,
	eSndRecordStateStartRecordingInPausedState,
	eSndRecordStatePauseRecording,
	eSndRecordStateResumeRecording,
	eSndRecordStateCancelRecording,
	eSndRecordStateError,

	eMaxSndRecordState
};

//! \public Media request for callback when processed MediaProcessor types
enum EMediaInputType
{
	eMediaInputNone,
	eMediaInputAudioPkts,
	eMediaInputAudioOpus,
	eMediaInputAudioPcm,

	eMediaInputVideoPkts,  // video packets send
	eMediaInputVideoJpg,   // 320x240 jpg video play

	eMediaInputMixer,

	eMaxMediaInputType
};
