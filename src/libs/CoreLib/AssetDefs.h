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

#include <stdint.h>
#include <string>

enum EAssetType
{
	// these must exactly match file type definitions in <CoreLib/VxFileTypeMasks.h>
	eAssetTypeUnknown				= 0x00,
	eAssetTypePhoto					= 0x01,
	eAssetTypeAudio					= 0x02,
	eAssetTypeVideo					= 0x04,
	eAssetTypeDocument				= 0x08,
	eAssetTypeArchives				= 0x10,
	eAssetTypeExe					= 0x20,
	eAssetTypeOtherFiles			= 0x40,
	eAssetTypeDirectory				= 0x80,
	// these are specific to this application
    eAssetTypeThumbnail             = 0x0100,
    eAssetTypeChatText				= 0x0200,
    eAssetTypeChatFace				= 0x0400,
    eAssetTypeCamRecord             = 0x0800,

	eAssetTypeSessionOffer			= 0x1000
};

enum EAssetLocation
{
	eAssetLocUnknown				= 0x00,
	eAssetLocLibrary				= 0x01,
	eAssetLocShared					= 0x02,
	eAssetLocPersonalRec			= 0x04,
    eAssetLocThumbDirectory         = 0x08,
    eAssetLocCamRecord              = 0x10
};

enum EAssetAction
{
	eAssetActionUnknown				= 0,
	eAssetActionDeleteFile			= 1,
	eAssetActionShreadFile			= 2,
	eAssetActionAddToAssetMgr		= 3,
	eAssetActionRemoveFromAssetMgr	= 4,	
	eAssetActionUpdateAsset			= 5,
	eAssetActionAddAssetAndSend		= 6,
	eAssetActionAssetSend			= 7,
	eAssetActionAssetResend			= 8,

	eAssetActionAddToShare			= 9,
	eAssetActionRemoveFromShare		= 10,
	eAssetActionAddToLibrary		= 11,
	eAssetActionRemoveFromLibrary	= 12,
	eAssetActionAddToHistory		= 13,
	eAssetActionRemoveFromHistory	= 14,

	eAssetActionRecordBegin			= 15,
	eAssetActionRecordPause			= 16,
	eAssetActionRecordResume		= 17,
	eAssetActionRecordProgress		= 18,
	eAssetActionRecordEnd			= 19,
	eAssetActionRecordCancel		= 20,

	eAssetActionPlayBegin			= 21,
	eAssetActionPlayOneFrame		= 22,
	eAssetActionPlayPause			= 23,
	eAssetActionPlayResume			= 24,
	eAssetActionPlayProgress		= 25,
	eAssetActionPlayEnd				= 26,
	eAssetActionPlayCancel			= 27,

	eAssetActionTxBegin				= 28,
	eAssetActionTxProgress			= 29,
	eAssetActionTxSuccess			= 30,
	eAssetActionTxError				= 31,
	eAssetActionTxCancel			= 32,
	eAssetActionTxPermission		= 33,

	eAssetActionRxBegin				= 34,
	eAssetActionRxProgress			= 35,
	eAssetActionRxSuccess			= 36,
	eAssetActionRxError				= 37,
	eAssetActionRxCancel			= 38,
	eAssetActionRxPermission		= 39,

	eAssetActionRxNotifyNewMsg		= 40,
	eAssetActionRxViewingMsg		= 41,
};

enum EAssetSendState
{
	eAssetSendStateNone     = 0,
	eAssetSendStateQueued,
	eAssetSendStateTxProgress,
	eAssetSendStateRxProgress,
	eAssetSendStateTxSuccess,
	eAssetSendStateTxFail,
	eAssetSendStateRxSuccess,
	eAssetSendStateRxFail,
	eAssetSendStateTxPermissionErr,
	eAssetSendStateRxPermissionErr,
	
	eMaxAssetSendState	
};

bool		VxIsValidAssetType( enum EAssetType assetType );
EAssetType	VxFileNameToAssetType( std::string fileName );
uint8_t		VxFileNameToFileType( std::string fileName );
EAssetType	VxFileTypeToAssetType( uint8_t fileType );

const char* DescribeAssetType( enum EAssetType assetType );
const char* DescribeAssetAction( enum EAssetAction assetAction );
const char* DescribeAssetSendState( enum EAssetSendState sendState );
