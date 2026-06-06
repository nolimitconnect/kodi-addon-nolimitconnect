#pragma once
//============================================================================
// Copyright (C) 2016 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

enum EXferDirection
{
    eXferDirectionNone,
    eXferDirectionRx,
    eXferDirectionTx,

    eMaxXferDirection
};

enum EXferError
{
	eXferErrorNone				= 0,
	eXferErrorDisconnected,
	eXferErrorPermission,
	eXferErrorFileNotFound,
	eXferErrorCanceled,
	eXferErrorBadParam,
	eXferErrorAtSrc,
	eXferErrorBusy,
	eXferErrorAlreadyDownloading,
	eXferErrorAlreadyDownloaded,
	eXferErrorAlreadyUploading,
	eXferErrorFileCreateError,
	eXferErrorFileOpenAppendError,
	eXferErrorFileOpenError,
	eXferErrorFileSeekError,
	eXferErrorFileReadError,
	eXferErrorFileWriteError,
	eXferErrorFileMoveError,

	eMaxXferError
};

enum EXferState
{
    eXferStateUnknown,

    eXferStateUploadNotStarted,
    eXferStateWaitingOfferResponse,
    eXferStateInUploadQue,
    eXferStateBeginUpload,
    eXferStateInUploadXfer,
    eXferStateCompletedUpload,
    eXferStateUserCanceledUpload,
    eXferStateUploadOfferRejected,
    eXferStateUploadError,

    eXferStateDownloadNotStarted,
    eXferStateInDownloadQue,
    eXferStateBeginDownload,
    eXferStateInDownloadXfer,
    eXferStateCompletedDownload,
    eXferStateUserCanceledDownload,
    eXferStateDownloadError,

    eXferStateStreaming,
    eXferStateStreamStopped,

    eMaxXferState
};

const char* DescribeXferDirection( enum EXferDirection xferDir );
const char* DescribeXferError( enum EXferError xferErr );
const char* DescribeXferState( enum EXferState xferState );
