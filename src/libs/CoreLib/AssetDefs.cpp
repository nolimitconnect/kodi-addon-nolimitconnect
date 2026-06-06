//============================================================================
// Copyright (C) 2021 Brett R. Jones
// 
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetDefs.h"
#include "VxFileIsTypeFunctions.h"
#include "VxDebug.h"
#include "VxFileUtil.h"

//============================================================================
EAssetType VxFileNameToAssetType( std::string fileName )
{
    if( fileName.empty() )
    {
        LogMsg( LOG_ERROR, "VxFileNameToAssetType empty file name" );
        return eAssetTypeUnknown;

    }
    else if( VxIsPhotoFile( fileName ) )
    {
        return eAssetTypePhoto;
    }
    else if( VxIsAudioFile( fileName ) )
    {
        return eAssetTypeAudio;
    }
    else if( VxIsVideoFile( fileName ) )
    {
        return eAssetTypeVideo;
    }
    else if( VxIsDocumentFile( fileName ) )
    {
        return eAssetTypeDocument;
    }
    else if( VxIsArcOrCDImageFile( fileName ) )
    {
        return eAssetTypeArchives;
    }
    else if( VxIsExecutableFile( fileName ) )
    {
        LogMsg( LOG_WARN, "VxFileNameToAssetType is exe type" );
        return eAssetTypeExe;
    }
    else if( VxIsThumbnailFile( fileName ) )
    {
        return eAssetTypeThumbnail;
    }
    else if( VxFileUtil::fileExists( fileName.c_str() ) )
    {
        LogMsg( LOG_WARN, "VxFileNameToAssetType unrecognized file but file exists" );
        return eAssetTypeOtherFiles;
    }
    else
    {
        LogMsg( LOG_WARN, "VxFileNameToAssetType unknown file type" );
        return eAssetTypeUnknown;
    }
}

//============================================================================
uint8_t VxFileNameToFileType( std::string fileName )
{
    return (uint8_t)VxFileNameToAssetType( fileName );
}

//============================================================================
EAssetType VxFileTypeToAssetType( uint8_t fileType )
{
    if( !fileType )
    {
        LogMsg( LOG_WARN, "VxFileTypeToAssetType no file type is set" );
        return eAssetTypeUnknown;
    }

    if( eAssetTypePhoto & fileType )
    {
        return eAssetTypePhoto;
    }
    else if( eAssetTypeAudio & fileType )
    {
        return eAssetTypeAudio;
    }
    else if( eAssetTypeVideo & fileType )
    {
        return eAssetTypeVideo;
    }
    else if( eAssetTypeDocument & fileType )
    {
        return eAssetTypeDocument;
    }
    else if( eAssetTypeArchives & fileType )
    {
        return eAssetTypeArchives;
    }
    else if( eAssetTypeExe & fileType )
    {
        LogMsg( LOG_WARN, "VxFileTypeToAssetType is exe type" );
        return eAssetTypeExe;
    }
    else if( eAssetTypeOtherFiles & fileType )
    {
        LogMsg( LOG_WARN, "VxFileTypeToAssetType is other files type" );
        return eAssetTypeOtherFiles;
    }
    else if( eAssetTypeDirectory & fileType )
    {
        LogMsg( LOG_WARN, "VxFileTypeToAssetType is directory type" );
        return eAssetTypeDirectory;
    }
    else
    {
        return eAssetTypeUnknown;
    }
}

//============================================================================
const char* DescribeAssetAction( enum EAssetAction assetAction )
{
    if( assetAction < 0 || eAssetActionRxViewingMsg < assetAction )
    {
        return "DescribeAssetAction BAD PARAM";
    }
    switch( assetAction )
    {
    case eAssetActionUnknown:               return "Asset Action Unknown";
    case eAssetActionDeleteFile:            return "Asset Action Delete File";
    case eAssetActionShreadFile:            return "Asset Action Shread File";
    case eAssetActionAddToAssetMgr:         return "Asset Action Add To Asset Manager";
    case eAssetActionRemoveFromAssetMgr:    return "Asset Action Remove From Asset Manager";
    case eAssetActionUpdateAsset:           return "Asset Action Update Asset";
    case eAssetActionAddAssetAndSend:       return "Asset Action Add Asset And Send";
    case eAssetActionAssetSend:             return "Asset Action Asset Send";
    case eAssetActionAssetResend:           return "Asset Action Asset Resend";
    case eAssetActionAddToShare:            return "Asset Action Add To Share";
    case eAssetActionRemoveFromShare:       return "Asset Action Remove From Share";
    case eAssetActionAddToLibrary:          return "Asset Action Add To Library";
    case eAssetActionRemoveFromLibrary:     return "Asset Action Remove From Library";
    case eAssetActionAddToHistory:          return "Asset Action Add To History";
    case eAssetActionRemoveFromHistory:     return "Asset Action Remove From History";
    case eAssetActionRecordBegin:           return "Asset Action Record Begin";
    case eAssetActionRecordPause:           return "Asset Action Record Pause";
    case eAssetActionRecordResume:          return "Asset Action Record Resume";
    case eAssetActionRecordProgress:        return "Asset Action Record Progress";
    case eAssetActionRecordEnd:             return "Asset Action Record End";
    case eAssetActionRecordCancel:          return "Asset Action Record Cancel";
    case eAssetActionPlayBegin:             return "Asset Action Play Begin";
    case eAssetActionPlayOneFrame:          return "Asset Action Play One Frame";
    case eAssetActionPlayPause:             return "Asset Action Play Pause";
    case eAssetActionPlayResume:            return "Asset Action Play Resume";
    case eAssetActionPlayProgress:          return "Asset Action Play Progress";
    case eAssetActionPlayEnd:               return "Asset Action Play End";
    case eAssetActionPlayCancel:            return "Asset Action Play Cancel";
    case eAssetActionTxBegin:               return "Asset Action Tx Begin";
    case eAssetActionTxProgress:            return "Asset Action Tx Progress";
    case eAssetActionTxSuccess:             return "Asset Action Tx Success";
    case eAssetActionTxError:               return "Asset Action Tx Error";
    case eAssetActionTxCancel:              return "Asset Action Tx Cancel";
    case eAssetActionTxPermission:          return "Asset Action Tx Permission";
    case eAssetActionRxBegin:               return "Asset Action Rx Begin";
    case eAssetActionRxProgress:            return "Asset Action Rx Progress";
    case eAssetActionRxSuccess:             return "Asset Action Rx Success";
    case eAssetActionRxError:               return "Asset Action Rx Error";
    case eAssetActionRxCancel:              return "Asset Action Rx Cancel";
    case eAssetActionRxPermission:          return "Asset Action Rx Permission";
    case eAssetActionRxNotifyNewMsg:        return "Asset Action Rx Notify New Msg";
    case eAssetActionRxViewingMsg:          return "Asset Action Rx Viewing Msg";

    default:
        LogMsg( LOG_ERROR, "Asset Action More than one flag 0x%X", assetAction );
        vx_assert( false );
        return "Asset Action Invalid";
    }
}

//============================================================================
const char* DescribeAssetType( EAssetType assetType )
{
    if( assetType < 0 || eAssetTypeCamRecord < assetType )
    {
        return "DescribeAssetType BAD PARAM";
    }

    if( !assetType )
    {
        LogMsg( LOG_WARN, "VxFileTypeToAssetType no assetType is set" );
        return "Asset Type Unknown";
    }

    switch( assetType )
    {
    case eAssetTypePhoto:               return "Asset Type Photo";
    case eAssetTypeAudio:               return "Asset Type Audio";
    case eAssetTypeVideo:               return "Asset Type Video";
    case eAssetTypeDocument:            return "Asset Type Document";
    case eAssetTypeArchives:            return "Asset Type Archive";
    case eAssetTypeExe:                 return "Asset Type Executable";
    case eAssetTypeOtherFiles:          return "Asset Type OtherFiles";
    case eAssetTypeDirectory:           return "Asset Type Directory";
    case eAssetTypeThumbnail:           return "Asset Type Thumbnail";
    case eAssetTypeChatText:            return "Asset Type Chat Text";
    case eAssetTypeChatFace:            return "Asset Type Chat Face";
    case eAssetTypeCamRecord:           return "Asset Type Cam Record";

    default:
        LogMsg( LOG_ERROR, "Asset Type More than one flag 0x%X", assetType );
        vx_assert( false );
        return "Asset Type Invalid";
    }
}

//============================================================================
const char* DescribeAssetSendState( enum EAssetSendState sendState )
{
    if( sendState < 0 || sendState >= eMaxAssetSendState )
    {
        return "DescribeAssetSendState BAD PARAM";
    }

    switch( sendState )
    {
    case eAssetSendStateNone:               return "eAssetSendStateNone";
    case eAssetSendStateQueued:             return "eAssetSendStateQueued";
    case eAssetSendStateTxProgress:         return "eAssetSendStateTxProgress";
    case eAssetSendStateRxProgress:         return "eAssetSendStateRxProgress";
    case eAssetSendStateTxSuccess:          return "eAssetSendStateTxSuccess";
    case eAssetSendStateTxFail:             return "eAssetSendStateTxFail";
    case eAssetSendStateRxSuccess:          return "eAssetSendStateRxSuccess";
    case eAssetSendStateRxFail:             return "eAssetSendStateRxFail";
    case eAssetSendStateTxPermissionErr:    return "eAssetSendStateTxPermissionErr";
    case eAssetSendStateRxPermissionErr:    return "eAssetSendStateRxPermissionErr";

    default:
        LogMsg( LOG_ERROR, "DescribeAssetSendState INVALID %d", sendState );
        vx_assert( false );
        return "Asset Send State Invalid";
    }
}

//============================================================================
bool VxIsValidAssetType( EAssetType assetType )
{
    switch( assetType )
    {
    case eAssetTypePhoto:
    case eAssetTypeAudio:
    case eAssetTypeVideo:
    case eAssetTypeDocument:
    case eAssetTypeArchives:
    case eAssetTypeExe:
    case eAssetTypeOtherFiles:
    case eAssetTypeDirectory:
    case eAssetTypeThumbnail:
    case eAssetTypeChatText:
    case eAssetTypeChatFace:
    case eAssetTypeCamRecord:
        return true;

    default:
        LogMsg( LOG_ERROR, "VxIsValidAssetType false 0x%X", assetType );
        vx_assert( false );
        return false;
    }
}
