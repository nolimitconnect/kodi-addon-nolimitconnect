
//============================================================================
// Copyright (C) 2009 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "P2PEngine.h"
#include "GuiInterface/IToGui.h"

#include "P2PConnectList.h"
#include "../NetServices/NetServicesMgr.h"

#include <AssetMgr/AssetInfo.h>
#include <AssetMgr/AssetMgr.h>
#include <BigListLib/BigListInfo.h>
#include <BlobXferMgr/BlobMgr.h>
#include <FileMgr/FileMgr.h>
#include <FriendRequestMgr/FriendRequestMgr.h>

#include <Network/NetworkMgr.h>

#include <NetworkMonitor/NetworkMonitor.h>
#include <NetworkTest/IsPortOpenTest.h>
#include <NetworkTest/RunUrlAction.h>

#include <MediaProcessor/MediaProcessor.h>
#include <MediaToolsLib/MediaTools.h>

#include <OfferBase/OfferMgr.h>

#include <Plugins/FileToXfer.h>
#include <Plugins/PluginBase.h>
#include <Plugins/PluginFileShareServer.h>
#include <Plugins/PluginFriendRequest.h>
#include <Plugins/PluginNetServices.h>
#include <Plugins/PluginNetworkHost.h>
#include <Plugins/PluginMgr.h>

#include <HostServerJoinMgr/HostServerJoinMgr.h>
#include <SendQueue/SendQueueMgr.h>
#include <UserJoinMgr/UserJoinMgr.h>

#include <UrlMgr/UrlMgr.h>

#include <CoreLib/Invite.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxParse.h>
#include <CoreLib/VxPtopUrl.h>

#include <NetLib/VxGetRandomPort.h>
#include <NetLib/VxPeerMgr.h>
#include <NetLib/VxSktBase.h>

#include <PktLib/PktsRandConnect.h>

#include <string.h>
#include <stdio.h>

//============================================================================
void P2PEngine::assureUserSpecificDirIsSet( const char* checkReason )
{
	if( false == m_IsUserSpecificDirSet )
	{
		LogMsg( LOG_ERROR, "P2PEngine::assureUserSpecificDirIsSet %s", checkReason );
		vx_assert( false );
	}
}

//============================================================================
uint64_t P2PEngine::fromGuiGetDiskFreeSpace( const char* dir  ) 
{
	if( dir )
	{
		std::string storageFile = dir;
		std::string storageDir;
		std::string fileName;
		VxFileUtil::seperatePathAndFile( storageFile, storageDir, fileName );
		if( !storageDir.empty() )
		{
			return VxFileUtil::getDiskFreeSpace( storageDir.c_str() );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		std::string incompleteDir = VxGetIncompleteDirectory();
		return VxFileUtil::getDiskFreeSpace( incompleteDir.c_str() );
	}
}

//============================================================================
uint64_t P2PEngine::fromGuiClearCache( ECacheType cacheType )
{
	if( eCacheTypeThumbnail == cacheType )
	{
		return m_ThumbMgr.fromGuiClearCache( cacheType );
	}

	return 0;
}

//============================================================================
bool P2PEngine::fromGuiDeleteUser( VxGUID& onlineId )
{
	return getBigListMgr().fromGuiDeleteUser( onlineId );
}

//============================================================================
void P2PEngine::fromGuiOnlineNameChanged( const char* newOnlineName )
{
	lockAnnouncePktAccess();
	m_PktAnn.setOnlineName( newOnlineName );
	unlockAnnouncePktAccess();
	doPktAnnHasChanged( false );
}

//============================================================================
void P2PEngine::fromGuiMoodMessageChanged( const char* newMoodMessage )
{
	lockAnnouncePktAccess();
	m_PktAnn.setOnlineDescription( newMoodMessage );
	unlockAnnouncePktAccess();
	doPktAnnHasChanged( false );
}

//============================================================================
void P2PEngine::fromGuiIdentPersonalInfoChanged( int age, int gender, int language, int preferredContent )
{
	lockAnnouncePktAccess();
    m_PktAnn.setAgeType( (EAgeType)age );
    m_PktAnn.setGender( (EGenderType)gender );
    m_PktAnn.setPrimaryLanguage( (ELanguageType)language );
    m_PktAnn.setPreferredContent( (EContentRating)preferredContent );
	unlockAnnouncePktAccess();
    doPktAnnHasChanged( false );
}

//============================================================================
void P2PEngine::fromGuiSetUserHasProfilePicture( bool haveProfilePick )
{
	if( m_PktAnn.hasAboutMeContent() != haveProfilePick )
	{
		lockAnnouncePktAccess();
		m_PktAnn.setHasAboutMeContent( haveProfilePick );
		unlockAnnouncePktAccess();
		doPktAnnHasChanged( false );
	}
}

//============================================================================
void P2PEngine::fromGuiUpdateMyIdent( VxNetIdent* netIdent, bool permissionAndStatsOnly )
{
	lockAnnouncePktAccess();
	if( permissionAndStatsOnly )
	{	
		memcpy( m_PktAnn.getPluginPermissions(), netIdent->getPluginPermissions(), PERMISSION_ARRAY_SIZE );
		m_PktAnn.setTruthAcceptCount( netIdent->getTruthAcceptCount() );
		m_PktAnn.setTruthRejectCount( netIdent->getTruthRejectCount() );
		m_PktAnn.setDareAcceptCount( netIdent->getDareAcceptCount() );
		m_PktAnn.setDareRejectCount( netIdent->getDareRejectCount() );
	}
	else
	{
		memcpy( (VxNetIdent*)&m_PktAnn, netIdent, sizeof( VxNetIdent ));
	}

	unlockAnnouncePktAccess();

	doPktAnnHasChanged( false );
}

//============================================================================
void P2PEngine::fromGuiSetIdentHasTextOffers( VxGUID& onlineId, bool hasTextOffers )
{
	BigListInfo * bigListInfo = m_BigListMgr.findBigListInfo( onlineId );
	if( bigListInfo )
	{
		if( bigListInfo->getHasTextOffers() != hasTextOffers )
		{
			bigListInfo->setHasTextOffers( hasTextOffers );
			m_BigListMgr.updateBigListDatabase( bigListInfo, getNetworkMgr().getNetworkKey().c_str() );
		}
	}
}

//============================================================================
void P2PEngine::fromGuiQueryMyIdent( VxNetIdent* poRetIdent )
{
	lockAnnouncePktAccess();
	memcpy( poRetIdent, (VxNetIdent*)&m_PktAnn, sizeof( VxNetIdent ) );
	unlockAnnouncePktAccess();
}

//============================================================================
//=== user gui input actions ===//
//============================================================================
//! called after processed HandleOrientationEvent for derived classes to override
bool P2PEngine::fromGuiOrientationEvent( float f32RotX, float f32RotY, float f32RotZ  )
{
	return false;
}

//============================================================================
//! called after processed HandleMouseEvent for derived classes to override
bool P2PEngine::fromGuiMouseEvent( enum EMouseButtonType eMouseButType, EMouseEventType eMouseEventType, int iMouseXPos, int iMouseYPos  )
{
	return false;
}

//============================================================================
//! called after processed HandleMouseWheel for derived classes to override
bool P2PEngine::fromGuiMouseWheel( float f32MouseWheelDist )
{
	return false;
}

//============================================================================
//! called after processed HandleKeyEvent for derived classes to override
bool P2PEngine::fromGuiKeyEvent( enum EKeyEventType eKeyEventType, EKeyCode eKey, int iFlags )
{
	return false;
}

//============================================================================
void P2PEngine::fromGuiNativeGlInit( void )
{
}

//============================================================================
//! resize window
void P2PEngine::fromGuiNativeGlResize( int w, int h )
{
}

//============================================================================
//! called to render the next frame
int  P2PEngine::fromGuiNativeGlRender( void )
{
	return 0;
}

//============================================================================
//! called to pause the render loop
void P2PEngine::fromGuiNativeGlPauseRender( void )
{
}

//============================================================================
//! called to resume the render loop
void P2PEngine::fromGuiNativeGlResumeRender( void )
{
}

//============================================================================
//! called when game window is being destroyed
void P2PEngine::fromGuiNativeGlDestroy( void )
{
}

//============================================================================
bool P2PEngine::fromGuiSndRecord( enum ESndRecordState eRecState, VxGUID& feedId, const char* fileName )
{
	return m_MediaProcessor.getMediaTools().fromGuiSndRecord( eRecState, feedId, fileName );
}

//============================================================================
bool P2PEngine::fromGuiAssetAction( enum EAssetAction assetAction, AssetBaseInfo& assetInfo, int pos0to100000 )
{
	AssetBaseInfo* createdAssetInfo = nullptr;
	if( eAssetActionAddAssetAndSend == assetAction && assetInfo.getPluginType() == ePluginTypePersonalRecorder )
	{
		// never send personal recordings
		assetAction = eAssetActionAddToAssetMgr;
	}

	if( eAssetActionAddToAssetMgr == assetAction )
	{
		return m_AssetMgr.addAsset( assetInfo, createdAssetInfo );
	}
	else if( eAssetActionAddAssetAndSend == assetAction )
	{
		assetInfo.setAssetSendState( eAssetSendStateTxProgress );
		bool result = m_AssetMgr.addAsset( assetInfo, createdAssetInfo );
        if( false == result )
        {
            LogMsg( LOG_ERROR, "PEngine::fromGuiAssetAction failed to add asset" );
			return false;
        }

		if( createdAssetInfo )
		{
			return fromGuiSendAsset( *createdAssetInfo );
		}	
	}
	else if( eAssetActionAssetSend == assetAction )
	{
		assetInfo.setAssetSendState( eAssetSendStateTxProgress );
		IToGui::getIToGui().toGuiAssetAction( eAssetActionTxBegin, assetInfo.getAssetUniqueId(), 0 );
		return fromGuiSendAsset( assetInfo );
	}
	else if( eAssetActionAssetResend == assetAction )
	{
		assetInfo.setAssetSendState( eAssetSendStateTxProgress );
		IToGui::getIToGui().toGuiAssetAction( eAssetActionTxBegin, assetInfo.getAssetUniqueId(), 0 );
		return fromGuiSendAsset( assetInfo );
	}
	else if( eAssetActionShreadFile == assetAction )
	{
		bool isFileAsset = assetInfo.isFileAsset();
		std::string fileNameAndPath = assetInfo.getAssetNameAndPath();
		if( isFileAsset && !fileNameAndPath.empty() )
		{
			m_AssetMgr.deleteFile( fileNameAndPath, true );
		}

		return true;
	}
	else if( eAssetActionRemoveFromAssetMgr == assetAction )
	{
		m_AssetMgr.removeAsset( assetInfo.getAssetUniqueId() );
		return true;
	}
	else if( eAssetActionPlayOneFrame == assetAction )
	{
		// takes too much time doing all the file io so queue the command instead
		getFromGuiMgr().fromGuiPlayOneFrame( assetInfo );
		return true;
	}

	return m_MediaProcessor.getMediaTools().fromGuiAssetAction( assetInfo, assetAction, pos0to100000 );
}

//============================================================================
bool P2PEngine::fromGuiQueueAssetAction( enum EAssetAction assetAction, AssetBaseInfo& assetInfo, int pos0to100000 )
{
	assetInfo.setAssetSendState( eAssetSendStateQueued );
	AssetBaseInfo* createdAssetInfo = nullptr;
	bool result = getSendQueueMgr().updateSendQueue( assetInfo.getSendToId(), assetInfo.getAssetUniqueId(), eSendQueStateWaiting );
	if( result )
	{		
		result = m_AssetMgr.addAsset( assetInfo, createdAssetInfo );
	}

	return result;
}

//============================================================================
bool P2PEngine::fromGuiAssetAction( enum EPluginType pluginType, enum EAssetAction assetAction, VxGUID& assetId, int pos0to100000 )
{
	if( eAssetActionAddToAssetMgr == assetAction )
	{
		LogMsg( LOG_ERROR, "fromGuiAssetAction Insufficient asset info to add asset" );
		return false;
	}

	//TODO figure out lock protection here... should we make copy? would be slow
	AssetInfo* assetInfo =  dynamic_cast<AssetInfo*>(m_AssetMgr.findAsset( assetId ));
	if( assetInfo )
	{
		return fromGuiAssetAction( assetAction, *assetInfo, pos0to100000 );
	}

	return false;
}

//============================================================================
bool P2PEngine::fromGuiVideoRecord( EVideoRecordState eRecState, VxGUID& feedId, const char* fileName   )
{
	return m_MediaProcessor.getMediaTools().fromGuiVideoRecord( eRecState, feedId, fileName );
}

//! play video or audio file
//============================================================================
bool P2PEngine::fromGuiPlayLocalMedia( const char* fileName, const char* fileNameAndPath, uint64_t fileLen, uint8_t fileType, int pos0to100000 )
{
	return fromGuiPlayLocalMedia( fileName, fileNameAndPath, fileLen, fileType, VxGUID::nullVxGUID(), pos0to100000 );
}

//============================================================================
bool P2PEngine::fromGuiPlayLocalMedia( const char*  fileName, const char* fileNameAndPath, uint64_t fileLen, uint8_t fileType, VxGUID assetId, int pos0to100000  )
{
    std::string fileNameStr = fileNameAndPath;
    bool result = true;
    EAssetType assetType = VxFileTypeToAssetType( fileType );
    if( !fileNameStr.empty() &&  fileLen && eAssetTypeUnknown != assetType )
    {
        AssetInfo * assetInfo =  dynamic_cast<AssetInfo*>(getAssetMgr().findAsset( fileNameStr ));
        if( 0 == assetInfo )
        {
			if( assetId.isValid() )
			{
				assetInfo = dynamic_cast<AssetInfo*>(getAssetMgr().addAssetFile( assetType, fileName, fileNameAndPath, fileLen, assetId ));
			}
			else
			{
				assetInfo = dynamic_cast<AssetInfo*>(getAssetMgr().addAssetFile( assetType, fileName, fileNameAndPath, fileLen ));
			}

            assetInfo->setPlayPosition( pos0to100000 );
        }

        if( 0 == assetInfo )
        {
            LogMsg( LOG_ERROR, "P2PEngine::fromGuiPlayLocalMedia INVALID PARAM" );
            result = false;
        }
        else
        {
            if( eAssetTypeVideo == assetInfo->getAssetType() )
            {
                if( fromGuiIsNoLimitVideoFile( fileName ) )
                {
                    fromGuiAssetAction( eAssetActionPlayBegin, *assetInfo, assetInfo->getPlayPosition() );
                }
                else
                {
                    IToGui::getIToGui().toGuiPlayNlcMedia( assetInfo );
                }
            }
            else if( eAssetTypeAudio == assetInfo->getAssetType() )
            {
                if( fromGuiIsNoLimitAudioFile( fileName ) ) 
                {
                    fromGuiAssetAction( eAssetActionPlayBegin, *assetInfo, assetInfo->getPlayPosition() );
                }
                else
                {
                    IToGui::getIToGui().toGuiPlayNlcMedia( assetInfo );
                }
            }
			else if( eAssetTypePhoto == assetInfo->getAssetType() )
			{
				IToGui::getIToGui().toGuiPlayNlcMedia( assetInfo );
			}
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "P2PEngine::fromGuiPlayLocalMedia INVALID PARAM" );
        result = false;
    }

    return result;
}

//============================================================================
//! update web page profile
void P2PEngine::fromGuiUpdateWebPageProfile(	const char*	pProfileDir,	// directory containing user profile	
												const char*	pGreeting,		// greeting text
												const char*	pAboutMe,		// about me text
												const char*	url1,			// favorite url 1
												const char*	url2,			// favorite url 2
												const char*	url3, 			// favorite url 3
                                                const char*	donation )	    // donation information
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiUpdateWebPageProfile" );
	std::string strWebPageHdr = "";
	StdStringFormat( strWebPageHdr, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html>\
<head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\" name=\"description\" content=\"%s :: About Me Web Page - Share introduction information about yourself in a personal web page served on your own device with access allowed or denied by friendship permission level\">\
<FONT COLOR=\"#0000FF\"><title>%s :: About Me</title></FONT></head><body bgcolor=\"#E3EFFF\"><h2><p align=\"center\"><font color=\"#007F00\">%s - %s - About Me</font></p></h2>",
		VxGetApplicationTitle(), VxGetApplicationTitle(), VxGetApplicationNameNoSpaces(), m_PktAnn.getOnlineName() );

	std::string strGreeting = "";
	if( strlen( pGreeting ))
	{
		StdStringFormat( strGreeting, "<h4><p align=\"center\">%s</p></h4>", pGreeting );
	}

	std::string strAbout		= "";
	std::string strPicLabel		= "<h3><p align=\"center\">My Picture</p></h3>";
	std::string strPicture		= "<p align=\"center\"><IMG SRC = \"me.png\" width=\"320\" height=\"240\"><br></p>";
	std::string strFavWebsites	= "</h2><p align=\"center\"><font color=\"#007F00\">My Favorite Web Sites</font></p></h2>";
	std::string strUrl1			= "";
	std::string strUrl2			= "";
	std::string strUrl3			= "";
    std::string strDonationInfo = "</h2><p align=\"center\"><font color=\"#007F00\">Donation Information</font></p></h2>";
    std::string strDonation = "";

	if( 0 !=  strlen( pGreeting ) )
	{
		StdStringFormat( strGreeting, "<h4><p align=\"center\">%s</p></h4>", pGreeting );    		
	}

	if( 0 !=  strlen( pAboutMe ) )
	{
		StdStringFormat( strAbout,  "<h4><p align=\"center\">%s</p></h4>", pAboutMe );
	}

	if( 0 !=  strlen( url1 ) )
	{
		StdStringFormat( strUrl1, "<h5><p align=\"center\"><font color=\"#0000FF\"><a href=\"%s\">%s</a></font></p><h5>", url1, url1 );
	}

	if( 0 !=  strlen( url2 ) )
	{
		StdStringFormat( strUrl2, "<h5><p align=\"center\"><font color=\"#0000FF\"><a href=\"%s\">%s</a></font></p><h5>", url2, url2 );
	}

	if( 0 !=  strlen( url3 ) )
	{
		StdStringFormat( strUrl3, "<h5><p align=\"center\"><font color=\"#0000FF\"><a href=\"%s\">%s</a></font></p><h5>", url3, url3 );
	}

    if( 0 != strlen( donation ) )
    {
        StdStringFormat( strDonation, "<h4><p align=\"center\">%s</p></h4>", donation );
    }
	else
	{
		strDonationInfo = "";
	}

	std::string strTrailer = "</body></html>\r\r\r";

	std::string strWebPage;
    strWebPage = strWebPageHdr + strGreeting + strAbout + strPicLabel + strPicture + strFavWebsites + strUrl1 + strUrl2 + strUrl3 + strDonationInfo + strDonation + strTrailer;

	std::string strWebFile = pProfileDir;
	strWebFile += "index.htm";

	VxFileUtil::writeWholeFile( strWebFile.c_str(), (void *)strWebPage.c_str(), (uint32_t)(strWebPage.length() ) );
}

//============================================================================
bool P2PEngine::fromGuiStartPluginSession( enum EPluginType pluginType, VxGUID oOnlineId, VxGUID lclSessionId )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiStartPluginSession" );
	return m_PluginMgr.fromGuiStartPluginSession( pluginType, oOnlineId, lclSessionId );
}

//============================================================================
void P2PEngine::fromGuiStopPluginSession( enum EPluginType pluginType, VxGUID oOnlineId, VxGUID lclSessionId )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiStopPluginSession" );
	m_PluginMgr.fromGuiStopPluginSession( pluginType, oOnlineId, lclSessionId );
}

//============================================================================
bool P2PEngine::fromGuiIsPluginInSession( enum EPluginType pluginType,VxGUID& onlineId, VxGUID lclSessionId )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiIsPluginInSession" );
	if( ( false == m_IsUserSpecificDirSet ) || VxIsAppShuttingDown() )
	{
		// wait until some things are started
		return false;	
	}

	return m_PluginMgr.fromGuiIsPluginInSession( pluginType, onlineId, lclSessionId );
}

//============================================================================
void P2PEngine::fromGuiSetPluginPermission( enum EPluginType pluginType, enum EFriendState eFriendState )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiSetPluginPermission" );
	EFriendState eCurFriendState = m_PktAnn.getPluginPermission( pluginType );
	if( eCurFriendState != eFriendState )
	{
		m_PluginMgr.setPluginPermission( pluginType, eFriendState );
		lockAnnouncePktAccess();
		m_PktAnn.setPluginPermission( pluginType, eFriendState );
		unlockAnnouncePktAccess();
		doPktAnnHasChanged( false );
	}
}

//============================================================================
int P2PEngine::fromGuiGetPluginPermission( enum EPluginType pluginType )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiGetPluginPermission" );
	return m_PktAnn.getPluginPermission( pluginType );
}

//============================================================================
EPluginServerState P2PEngine::fromGuiGetPluginServerState( enum EPluginType pluginType )
{
	if( eFriendStateIgnore == m_PktAnn.getPluginPermission( pluginType ) )
	{
		return ePluginServerStateDisabled;
	}

	return m_PluginMgr.fromGuiIsPluginInSession( pluginType, getMyOnlineId() ) ? ePluginServerStateStarted : ePluginServerStateStopped;
}

//============================================================================
//! called with offer to create session.. return false if cannot connect
bool P2PEngine::fromGuiMakePluginOffer( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	offerInfo.setDestUserId( onlineId );
	VxNetIdent* netIdent = m_BigListMgr.findNetIdent( onlineId );
	PluginBase* poPlugin = m_PluginMgr.getPlugin( offerInfo.getPluginType() );
	if( netIdent && poPlugin )
	{
		return poPlugin->fromGuiMakePluginOffer( onlineId, offerInfo );
	}
	else
	{
		LogMsg(LOG_ERROR, "P2PEngine::fromGuiMakePluginOffer: poInfo not found VxGUID %s", onlineId.toHexString().c_str());
	}

	return false;
}

//============================================================================
//! handle reply to offer
bool P2PEngine::fromGuiToPluginOfferReply( VxGUID& onlineId, OfferBaseInfo& offerInfo )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiToPluginOfferReply" );
	if( VxIsAppShuttingDown() )
	{
		return false;
	}

	VxNetIdent* netIdent = m_BigListMgr.findNetIdent( onlineId );
	PluginBase* poPlugin = m_PluginMgr.getPlugin( offerInfo.getPluginType() );
	if( netIdent && poPlugin )
	{
		return poPlugin->fromGuiOfferReply( onlineId, offerInfo );
	}
	else
	{
		LogMsg( LOG_ERROR, "ERROR P2PEngine::fromGuiToPluginOfferReply invalid plugin or info" );
		return false;
	}
}

//============================================================================
EXferError P2PEngine::fromGuiFileXferControl( enum EPluginType pluginType, EXferAction xferAction, FileInfo& fileInfo )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiFileXferControl" );
	PluginBase* poPlugin = m_PluginMgr.getPlugin( pluginType );
	if( poPlugin )
	{
		return poPlugin->fromGuiFileXferControl( fileInfo.getOnlineId(), xferAction, fileInfo );
	}
	else
	{
		LogMsg( LOG_ERROR, "ERROR P2PEngine::fromGuiFileXferControl invalid plugin" );
		return eXferErrorBadParam;
	}
}

//============================================================================
bool P2PEngine::fromGuiInstMsg( enum EPluginType	pluginType, VxGUID&	onlineId, const char* pMsg )
{
	//assureUserSpecificDirIsSet( "P2PEngine::fromGuiInstMsg" );

	PluginBase* poPlugin = m_PluginMgr.getPlugin( pluginType );
	if( poPlugin )
	{
		return poPlugin->fromGuiInstMsg( onlineId, pMsg );
	}
	else
	{
		LogMsg( LOG_ERROR, "ERROR P2PEngine::fromGuiInstMsg invalid plugin" );
		return false;
	}
}

//============================================================================
bool P2PEngine::fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk )
{
	PluginBase* poPlugin = m_PluginMgr.getPlugin( ePluginTypePushToTalk );
	if( poPlugin )
	{
		return poPlugin->fromGuiPushToTalk( onlineId, enableTalk );
	}
	else
	{
		LogMsg( LOG_ERROR, "ERROR P2PEngine::fromGuiPushToTalk invalid plugin or user" );
		return false;
	}
}

//============================================================================
bool P2PEngine::isUserConnected( VxGUID& onlineId )
{
	if( onlineId == m_MyOnlineId )
	{
		// just assume connected because is ourself
		return true;
	}

	return m_ConnectIdListMgr.isUserOnline( onlineId );
}

//============================================================================
bool P2PEngine::isInternetAvailable( void )
{
	return m_NetStatusAccum.isInternetAvailable();
}

//============================================================================
bool P2PEngine::isNetworkOnline( void )
{ 
	return m_NetStatusAccum.isNetworkOnline(); 
}

//============================================================================
bool P2PEngine::isDirectConnectTested( void )
{
	return m_NetStatusAccum.isDirectConnectTested();
}

//============================================================================
bool P2PEngine::getIsMyHostServiceEnabled( enum EHostType hostService )
{
	switch( hostService )
	{
	case eHostTypeNetwork: return m_PktAnn.getPluginPermission( ePluginTypeHostNetwork ) != eFriendStateIgnore;
	case eHostTypeGroup: return m_PktAnn.getPluginPermission( ePluginTypeHostGroup ) != eFriendStateIgnore;
	case eHostTypeConnectTest: return m_PktAnn.getPluginPermission( ePluginTypeHostConnectTest ) != eFriendStateIgnore;
	case eHostTypeChatRoom: return m_PktAnn.getPluginPermission( ePluginTypeHostChatRoom ) != eFriendStateIgnore;
	case eHostTypeRandomConnect: return m_PktAnn.getPluginPermission( ePluginTypeHostRandomConnect ) != eFriendStateIgnore;
	default:
		break;
	}

	return false;
}

//============================================================================
bool P2PEngine::getHasAnyHostServiceEnabled( void )
{
	return m_PktAnn.getPluginPermission( ePluginTypeHostNetwork ) != eFriendStateIgnore ||
		m_PktAnn.getPluginPermission( ePluginTypeHostGroup ) != eFriendStateIgnore ||
		m_PktAnn.getPluginPermission( ePluginTypeHostConnectTest ) != eFriendStateIgnore ||
		m_PktAnn.getPluginPermission( ePluginTypeHostChatRoom ) != eFriendStateIgnore ||
		m_PktAnn.getPluginPermission( ePluginTypeHostRandomConnect ) != eFriendStateIgnore;
}

//============================================================================
bool P2PEngine::getHasAnyAnnonymousHostService( void )
{
	return m_PktAnn.getPluginPermission( ePluginTypeHostGroup ) == eFriendStateAnonymous ||
		m_PktAnn.getPluginPermission( ePluginTypeHostChatRoom ) == eFriendStateAnonymous ||
		m_PktAnn.getPluginPermission( ePluginTypeHostRandomConnect ) == eFriendStateAnonymous ||
		m_PktAnn.getPluginPermission( ePluginTypeHostNetwork ) == eFriendStateAnonymous ||
		m_PktAnn.getPluginPermission( ePluginTypeHostConnectTest ) == eFriendStateAnonymous;
}

//============================================================================
bool P2PEngine::getHasFixedIpAddress( void )
{
	bool ipv6 = getEngineSettings().getUseIpv6();
    if( eFirewallTestAssumeNoFirewall == getEngineSettings().getFirewallTestSetting() )
    {
        std::string externIp;
        getEngineSettings().getUserSpecifiedExternIpAddr( externIp, ipv6 );
        if( externIp.empty() )
        {
            LogMsg( LOG_ERROR, "%s Assume no Firewall set but external ip is empty", __func__ );
			return false;
        }

        return true;
    }

    return false;
}

//============================================================================
void P2PEngine::fromGuiMuteMicrophone(	bool muteMic )
{
	m_MediaProcessor.muteMicrophone( muteMic );
}

//============================================================================
bool P2PEngine::fromGuiIsMicrophoneMuted( void )
{
	return m_MediaProcessor.isMicrophoneMuted();
}

//============================================================================
void P2PEngine::fromGuiMuteSpeaker(	bool muteSpeaker )
{
	m_MediaProcessor.muteSpeaker( muteSpeaker );
}

//============================================================================
bool P2PEngine::fromGuiIsSpeakerMuted( void )
{
	return m_MediaProcessor.isSpeakerMuted();
}

//============================================================================
void P2PEngine::fromGuiWantMediaInput( VxGUID& onlineId, enum EMediaInputType mediaType, MediaCallbackInterface * callback, enum EMediaModule mediaModule, VxGUID& mediaSessionId, bool wantInput )
{
	if( false == VxIsAppShuttingDown() )
	{
		m_MediaProcessor.wantMediaInput( onlineId, mediaType, callback, mediaModule, mediaSessionId, wantInput );
	}
}

//============================================================================
void P2PEngine::fromGuiWantMediaInput( VxGUID& onlineId, enum EMediaInputType mediaType, enum EMediaModule mediaModule, VxGUID& mediaSessionId, bool wantInput )
{
	if( false == VxIsAppShuttingDown() )
	{
        m_MediaProcessor.wantMediaInput( onlineId, mediaType, this, mediaModule, mediaSessionId, wantInput );
	}
}

//============================================================================
bool P2PEngine::fromGuiChangeMyFriendshipToHim(	VxGUID&				onlineId, 
												enum EFriendState	myFriendshipToHim,
												enum EFriendState	hisFriendshipToMe )
{
	if( false == VxIsAppShuttingDown() )
	{
		if( !onlineId.isValid() )
		{
            LogMsg( LOG_ERROR, "%s invalid id", __func__ );
			return false;
		}

		BigListInfo * poInfo = m_BigListMgr.findBigListInfo( onlineId );
		if( poInfo )
		{
			if( onlineId != getMyOnlineId() )
			{
				updateIdentLists( poInfo );
			}
			
			EFriendState eOldFriendship = poInfo->getMyFriendshipToHim();
			poInfo->setMyFriendshipToHim( myFriendshipToHim );
			poInfo->setNeedSavedToDb( true );
			m_BigListMgr.updateVectorList( eOldFriendship, poInfo );

			BigListInfo* dummyBigListInfo = nullptr;
			EHostType hostType{ eHostTypeUnknown };
			m_BigListMgr.updatePktAnn( poInfo->getPktAnnounce(), &dummyBigListInfo, hostType, true );

            LogMsg( LOG_VERBOSE, "P2PEngine::%s SUCCESS changed %s friendship to %s", __func__,
				poInfo->getOnlineName(),
				poInfo->describeMyFriendshipToHim());

			getConnectIdListMgr().updateOnlineExclusion( onlineId, myFriendshipToHim == eFriendStateIgnore );

			m_ConnectionList.fromGuiChangeMyFriendshipToHim( onlineId, myFriendshipToHim, hisFriendshipToMe );
			return true;
		}
		else
		{
			LogMsg( LOG_ERROR, "P2PEngine::fromGuiChangeMyFriendshipToHim: FAILED find friend %s", onlineId.toOnlineIdString().c_str() );
		}
	}

	return false;
}

//============================================================================
void P2PEngine::fromGuiAdminViewHost( EPluginType pluginType, bool adminIsViewing )
{
	if( VxIsAppShuttingDown() )
	{
		return;
	}

	m_PluginMgr.fromGuiAdminViewHost( pluginType, adminIsViewing );
}

//============================================================================
void P2PEngine::fromGuiRelayPermissionCount( int userPermittedCount, int anonymousCount )
{
	m_PluginMgr.fromGuiRelayPermissionCount( userPermittedCount, anonymousCount );
}

//============================================================================
InetAddress P2PEngine::fromGuiGetMyIpAddress( void )
{
	bool ipv6 = getEngineSettings().getUseIpv6();
	return ipv6 ? VxGetMyGlobalIPv6Address() : VxGetDefaultIPv4Address();
}

//============================================================================
InetAddress P2PEngine::fromGuiGetMyIPv4Address( void )
{
	return VxGetDefaultIPv4Address();
}

//============================================================================
InetAddress P2PEngine::fromGuiGetMyIPv6Address( void )
{
	return VxGetMyGlobalIPv6Address();
}

//============================================================================
void P2PEngine::fromGuiCancelDownload( VxGUID& fileInstance )
{
	m_PluginMgr.getPlugin( ePluginTypeFileShareClient )->fromGuiCancelDownload( fileInstance );
	m_PluginMgr.getPlugin( ePluginTypePersonFileXfer )->fromGuiCancelDownload( fileInstance );
}

//============================================================================
void P2PEngine::fromGuiCancelUpload( VxGUID& fileInstance )
{
	m_PluginMgr.getPlugin( ePluginTypeFileShareServer )->fromGuiCancelUpload( fileInstance );
	m_PluginMgr.getPlugin( ePluginTypePersonFileXfer )->fromGuiCancelUpload( fileInstance );
}

//============================================================================
void P2PEngine::fromGuiGetNetSettings( NetSettings& netSettings )
{
	m_EngineSettings.getNetSettings( netSettings );
}

//============================================================================
void P2PEngine::updateMyPktAnnIpAddress( std::string ipAddr )
{
	if( ipAddr.empty() )
	{
		LogMsg( LOG_ERROR, "%s empty ip addr", __func__ );
		return;
	}

	EIpAddrType ipType;
	std::string origIpAddr;
	bool sameIpAddr{ false };
	if( getMyPktAnnounce().getOnlineIpAddress( origIpAddr, ipType ) &&
		origIpAddr == ipAddr )
	{
		LogMsg( LOG_DEBUG, "%s same ip %s", __func__, ipAddr.c_str() );
		sameIpAddr = true;
	}

	if( !sameIpAddr )
	{
		LogMsg( LOG_INFO, "%s updating IP from %s to %s", __func__, origIpAddr.empty() ? "<empty>" : origIpAddr.c_str(), ipAddr.c_str() );
		getMyPktAnnounce().setOnlineIpAddress( ipAddr.c_str() );
		setPktAnnLastModTime( GetTimeStampMs() );
	}

	updateMyNetworkServiceUrl( eHostTypeNetwork );
	updateMyNetworkServiceUrl( eHostTypeConnectTest );
}

//============================================================================
void P2PEngine::updateMyNetworkServiceUrl( EHostType hostType )
{
	if( hostType != eHostTypeNetwork && hostType != eHostTypeConnectTest )
	{
		LogMsg( LOG_SEVERE, "P2PEngine::%s INVALID host type %d", __func__, hostType );
		vx_assert( false );
		return;
	}

	if( getIsMyHostServiceEnabled( hostType ) )
    {
        // I have host service enabled
        std::string myUrl = getMyOnlineUrl();
		VxPtopUrl myPtopUrl( myUrl );
		// cannot assume that host url is same as my url when enabled but network settings has someone elses network host url
		std::string netServiceUrl;
		
		if( hostType != eHostTypeNetwork )
		{
			getEngineSettings().getNetworkHostUrl( netServiceUrl );
		}
		else
		{
			getEngineSettings().getConnectTestUrl( netServiceUrl );
		}		

		VxPtopUrl netServicePtopUrl( netServiceUrl );
		if( myPtopUrl.getHost() == netServicePtopUrl.getHost() )
		{
			VxGUID myOnlineId = getMyOnlineId();

			getConnectionMgr().updateMyEnabledHostUrl( hostType, myUrl, myOnlineId );
			;
			getUrlMgr().updateUrlCache( myUrl, myOnlineId );
			getHostUrlListMgr().updateHostUrl( hostType, myOnlineId, myUrl );
		}
    }
}

//============================================================================
void P2PEngine::fromGuiSetNetSettings( NetSettings& netSettings )
{
	m_EngineSettings.setNetSettings( netSettings );
}

//============================================================================
void P2PEngine::fromGuiSetRelaySettings( int userRelayMaxCnt, int systemRelayMaxCnt )
{
	m_EngineSettings.setMaxRelaysInUse( userRelayMaxCnt, systemRelayMaxCnt );
}

//============================================================================
void P2PEngine::fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings )
{
	m_PluginMgr.getPlugin(ePluginTypeFileShareServer)->fromGuiGetFileShareSettings( fileShareSettings );
}

//============================================================================
void P2PEngine::fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings )
{
	m_PluginMgr.getPlugin(ePluginTypeFileShareServer)->fromGuiSetFileShareSettings( fileShareSettings );
}

//============================================================================
bool P2PEngine::fromGuiTodGameActionSend( enum EPluginType	pluginType, VxGUID& onlineId, ETodGameAction todGameAction )
{
	PluginBase* poPlugin = m_PluginMgr.getPlugin( pluginType );
	if( poPlugin )
	{
		return poPlugin->fromGuiTodGameActionSend( onlineId, todGameAction );
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::fromGuiTodGameActionSend: could not locate idenitiy");
	}

	return false;
}

//============================================================================
bool P2PEngine::fromGuiTestCmd( enum ETestParam1		eTestParam1,
								int				testParam2, 
								const char*		testParam3 )
{
	bool result = false;
	switch( eTestParam1 )
	{
	case eTestParam1FullNetTest1:
		{
			//NetSettings netSettings;
			//m_EngineSettings.getNetSettings( netSettings );
			//fromGuiTestNetwork( netSettings );
		}
		break;

	case eTestParam1WhatsMyIp:
		{
			//void						queryWhatsMyIp( void );
		}
		break;

	case eTestParam1IsMyPortOpen:
		{
			//m_PluginNetServices.testIsMyPortOpen();
		}
		break;

	//case eTestParam1AnnounceNow:
	//	{
	//		m_NetServicesMgr.announceToHost( m_NetworkStateMachine.getHostIp(), m_NetworkStateMachine.getHostPort() );
	//	}
	//	break;

	case eTestParamSoundDelayTest:
		{
			// m_MediaProcessor.fromGuiSoundDelayTest();
		}
		break;

	default:
		LogMsg( LOG_INFO, "Unknown eTestParam1 %d", eTestParam1 );
	}

	return result;
}

//============================================================================
uint16_t P2PEngine::fromGuiGetRandomTcpPort( void )
{
	return VxGetRandomTcpPort();
}

/// Get url for this node
//============================================================================
void P2PEngine::fromGuiGetNodeUrl( std::string& nodeUrl )
{
    nodeUrl = getMyOnlineUrl();
}

//============================================================================
/// Get internet status
EInternetStatus P2PEngine::fromGuiGetInternetStatus( void )
{
    return getNetStatusAccum().getInternetStatus();
}

//============================================================================
/// Get network status
ENetAvailStatus P2PEngine::fromGuiGetNetAvailStatus( void )
{
    return getNetStatusAccum().getNetAvailStatus();
}

//============================================================================
void P2PEngine::fromGuiAnnounceHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl, bool fromThread )
{
	if( fromThread )
	{
		PluginBase* plugin = m_PluginMgr.findHostClientPlugin( adminId.getHostType() );
		if( plugin )
		{
			plugin->fromGuiAnnounceHost( adminId, sessionId, hostUrl );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for host %d", adminId.getHostType() );
			vx_assert( false );
		}
	}
	else
	{
		m_FromGuiMgr.fromGuiAnnounceHost( adminId, sessionId, hostUrl );
	}
}

//============================================================================
void P2PEngine::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl, bool fromThread )
{
	if( fromThread )
	{
		PluginBase* plugin = m_PluginMgr.findHostClientPlugin( adminId.getHostType() );
		if( plugin )
		{
			plugin->fromGuiJoinHost( adminId, sessionId, hostUrl );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for host %d", adminId.getHostType() );
			vx_assert( false );
		}
	}
	else
	{
		m_FromGuiMgr.fromGuiJoinHost( adminId, sessionId, hostUrl );
	}
}

//============================================================================
void P2PEngine::fromGuiLeaveHost( HostedId& adminId, bool fromThread )
{
	if( fromThread )
	{
		bool result = getUserJoinMgr().fromGuiLeaveHost( adminId );
		if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "P2PEngine::%s left Host %s result %d", __func__,
			DescribeHostType( adminId.getHostType() ), result );
	}
	else
	{
		m_FromGuiMgr.fromGuiLeaveHost( adminId );
	}
}

//============================================================================
void P2PEngine::fromGuiUnJoinHost( HostedId& adminId, bool fromThread )
{
	if( fromThread )
	{
		bool result = getUserJoinMgr().fromGuiUnJoinHost( adminId );
		if( LogEnabled( eLogHostJoin ) )LogModule( eLogHostJoin, LOG_VERBOSE, "P2PEngine::%s Unjoined Host %s result %d", __func__,
			DescribeHostType( adminId.getHostType() ), result );
	}
	else
	{
		m_FromGuiMgr.fromGuiUnJoinHost( adminId );
	}
}

//============================================================================
void P2PEngine::fromGuiSearchHost( enum EHostType hostType, SearchParams& searchParams, bool enable, bool fromThread )
{
	if( fromThread )
	{
		PluginBase* plugin = m_PluginMgr.findHostClientPlugin( hostType );
		if( plugin )
		{
			plugin->fromGuiSearchHost( hostType, searchParams, enable );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for host %d", hostType );
			vx_assert( false );
		}
	}
	else
	{
		m_FromGuiMgr.fromGuiSearchHost( hostType, searchParams, enable );
	}
}

//============================================================================
void P2PEngine::fromGuiSendAnnouncedList( enum EHostType hostType, VxGUID& sessionId )
{
	PluginBase* plugin = m_PluginMgr.findPlugin( ePluginTypeHostNetwork );
	if( plugin )
	{
		plugin->fromGuiSendAnnouncedList( hostType, sessionId );
	}
	else
	{
		LogMsg( LOG_ERROR, "Plugin not found for host %d", hostType );
		vx_assert( false );
	}
}

//============================================================================
void P2PEngine::fromGuiDisconnectFromUser( VxGUID& onlineId )
{
	getConnectIdListMgr().fromGuiDisconnectFromUser( onlineId );
}

//============================================================================
void P2PEngine::fromGuiRunIsPortOpenTest( uint16_t port )
{
    m_IsPortOpenTest.fromGuiRunIsPortOpenTest( port );
}

//============================================================================
void P2PEngine::fromGuiRunUrlAction( VxGUID& sessionId, const char* myUrl, const char* ptopUrl, ENetCmdType testType )
{
    getRunUrlAction().runUrlAction( sessionId, testType, ptopUrl, myUrl );
}

//============================================================================
bool P2PEngine::fromGuiBrowseFiles( VxGUID& appInstId, std::string& dir, uint8_t fileFilterMask )
{
	return getPluginFileShareServer().fromGuiBrowseFiles( appInstId, dir, fileFilterMask );
}

//============================================================================
// returns -1 if unknown else percent downloaded
int P2PEngine::fromGuiGetFileDownloadState( uint8_t* fileHashId )
{
	return getPluginFileShareServer().fromGuiGetFileDownloadState( fileHashId );
}

//============================================================================
bool P2PEngine::fromGuiSetFileIsShared( FileInfo& fileInfo, bool isShared )
{
	return getPluginFileShareServer().fromGuiSetFileIsShared( fileInfo, isShared );
}

//============================================================================
bool P2PEngine::fromGuiGetIsFileShared( FileInfo& fileInfo )
{
	return getPluginFileShareServer().fromGuiGetFileIsShared( fileInfo );
}

//============================================================================
bool P2PEngine::fromGuiSetFileIsInLibrary( FileInfo& fileInfo, bool isInLibrary )
{
	return getPluginLibraryServer().fromGuiSetFileIsInLibrary( fileInfo, isInLibrary );
}

//============================================================================
bool P2PEngine::fromGuiGetFileIsInLibrary( FileInfo& fileInfo )
{
	return getPluginLibraryServer().fromGuiGetFileIsInLibrary( fileInfo );
}

//============================================================================
void P2PEngine::fromGuiGetFileLibraryList( VxGUID& appInstId, uint8_t fileTypeFilter )
{
	getPluginLibraryServer().fromGuiGetFileLibraryList(	appInstId, fileTypeFilter );
}

//============================================================================
bool P2PEngine::fromGuiIsNoLimitVideoFile( const char* fileName )
{
	return m_MediaProcessor.getMediaTools().fromGuiIsNoLimitVideoFile( fileName );
}

//============================================================================
bool P2PEngine::fromGuiIsNoLimitAudioFile( const char* fileName )
{
	return m_MediaProcessor.getMediaTools().fromGuiIsNoLimitAudioFile( fileName );
}

//============================================================================
int P2PEngine::fromGuiDeleteFile( std::string fileName, bool shredFile )
{
	int result = -1;
	if( !fileName.empty() )
	{
		FILE* fileHandle = fopen( fileName.c_str(), "rb" );
		if( fileHandle )
		{
			fclose( fileHandle );
			result = 0;

			// tell plugins we are removing file
			m_PluginMgr.fromGuiDeleteFile( fileName, shredFile );

			// if exists as asset then announce asset removal
			AssetBaseInfo* assetInfo = getAssetMgr().findAsset( fileName );
			if( assetInfo )
			{
				getAssetMgr().removeAsset( assetInfo->getAssetUniqueId() );
			}

			// remove from library and shared files then delete the file
			getPluginFileShareServer().deleteFile( fileName.c_str(), shredFile );

			getToGui().toGuiFileDeleted( fileName );
		}
		else
		{
			result = VxGetLastError();
			if( 0 == result )
			{
				result = -1;
			}

			LogMsg( LOG_WARNING, "P2PEngine::fromGuiDeleteFile  file cannot be opened err %d file name %s", result, fileName.c_str() );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::fromGuiDeleteFile bad fileName param" );
	}

	return result;
}

//============================================================================
void P2PEngine::fromGuiQuerySessionHistory( GroupieId& groupieId )
{
	m_AssetMgr.fromGuiQuerySessionHistory( groupieId );
}

//============================================================================
bool P2PEngine::fromGuiSendAsset( AssetBaseInfo& assetInfo )
{
	return m_PluginMgr.fromGuiSendAsset( assetInfo );
}

//============================================================================
bool P2PEngine::fromGuiMultiSessionAction( enum EMSessionAction mSessionAction, VxGUID& onlineId, int pos0to100000, VxGUID lclSessionId )
{
	return m_PluginMgr.fromGuiMultiSessionAction( mSessionAction, onlineId, pos0to100000, lclSessionId );
}

//============================================================================
int P2PEngine::fromGuiGetAnnouncedHostCount( enum EHostType hostType )
{
	int count{ 0 };
	PluginNetworkHost* networkHost = dynamic_cast<PluginNetworkHost*>( m_PluginMgr.getPlugin( ePluginTypeHostNetwork ) );
	if( networkHost )
	{
		count = networkHost->getAnnouncedHostCount( hostType );
	}

	return count;
}

//============================================================================
int P2PEngine::fromGuiGetJoinedListCount( enum EPluginType pluginType )
{
	return getHostJoinMgr().fromGuiGetJoinedListCount( pluginType );
}

//============================================================================
EJoinState P2PEngine::fromGuiQueryJoinState( enum EHostType hostType, VxNetIdent& netIdent )
{
	return getHostJoinMgr().fromGuiQueryJoinState( hostType, netIdent );
}

//============================================================================
void P2PEngine::fromGuiListAction( enum EListAction listAction )
{
	m_PluginMgr.fromGuiListAction( listAction );
}

//============================================================================
std::string P2PEngine::fromGuiQueryDefaultUrl( enum EHostType hostType, bool ignoreMyself )
{
	if( !ignoreMyself )
	{
		if( eHostTypeNetwork == hostType )
		{
			if( (m_PktAnn.getPluginPermission( ePluginTypeHostNetwork ) != eFriendStateIgnore) &&
				isDirectConnectTested() )
			{
				// I am the network host
				return getMyOnlineUrl( hostType );
			}
		}

		if( eHostTypeChatRoom == hostType )
		{
			if( (m_PktAnn.getPluginPermission( ePluginTypeHostChatRoom ) != eFriendStateIgnore) &&
				isDirectConnectTested() )
			{
				// I am the chat room host
				return getMyOnlineUrl( hostType );
			}
		}

		if( eHostTypeGroup == hostType )
		{
			if( (m_PktAnn.getPluginPermission( ePluginTypeHostGroup ) != eFriendStateIgnore) &&
				isDirectConnectTested() )
			{
				// I am the group host
				return getMyOnlineUrl( hostType );
			}
		}

		if( eHostTypeRandomConnect == hostType )
		{
			if( (m_PktAnn.getPluginPermission( ePluginTypeHostRandomConnect ) != eFriendStateIgnore) &&
				isDirectConnectTested() )
			{
				// I am the random connect host
				return getMyOnlineUrl( hostType );
			}
		}
	}

	if( eHostTypeNetwork == hostType && !m_NetStatusAccum.hasDefaultNetworkKey() && m_NetStatusAccum.hasDefaultNetworkUrl() )
	{
		// do not allow connection to default network host with the wrong network key
		return "";
	}

	std::string defaultUrl = getEngineSettings().fromGuiQueryDefaultUrl( hostType );
    std::string resolvedUrl = getUrlMgr().resolveUrl( defaultUrl );
	// if the resolved ip is our external ip then we are the host. if does not have online id then set to ours
	// this is so can query ourself for hosts to join
	VxPtopUrl ptopUrl( resolvedUrl );
	if( !ptopUrl.getOnlineId().isValid() )
	{
		if( ptopUrl.getHost() == getNetStatusAccum().getExternalIpAddress() )
		{
			// we are the host
			resolvedUrl += "/";
			resolvedUrl += getMyOnlineId().toOnlineIdString();
		}
		else if( eHostTypeNetwork == hostType || eHostTypeConnectTest == hostType )
		{
			// if query host id was done we should have the online id for this host
			VxGUID hostOnlineId;
			if( getUrlMgr().lookupOnlineId( resolvedUrl, hostOnlineId ) )
			{
				if( hostOnlineId.isValid() )
				{
					resolvedUrl += "/";
					resolvedUrl += hostOnlineId.toOnlineIdString();
				}
			}
		}
	}

	ptopUrl.setUrlHostType( hostType );
	Invite::appendHostTypeSuffix( hostType, resolvedUrl );
	return resolvedUrl;
}

//============================================================================
bool P2PEngine::fromGuiQueryIdentity( std::string& url, VxNetIdent& retNetIdent, bool requestIdentityIfUnknown )
{
	bool result{ false };
	VxPtopUrl ptopUrl( url );
	if( ptopUrl.isValid() )
	{
		if( getMyOnlineId() == ptopUrl.getOnlineId() )
		{
			retNetIdent = *getMyPktAnnounce().getVxNetIdent();
			return true;
		}

		BigListInfo* bigListInfo = m_BigListMgr.findBigListInfo( ptopUrl.getOnlineId() );
		if( bigListInfo )
		{
			retNetIdent = *bigListInfo->getVxNetIdent();
			result = true;
		}

		if( !result && requestIdentityIfUnknown )
		{
			// connect to url just to get identity
			getHostUrlListMgr().requestIdentity( ptopUrl.getUrl() );
		}
	}

	return result;
}

//============================================================================
bool P2PEngine::fromGuiQueryIdentity( VxGUID onlineId, VxNetIdent& retNetIdent )
{
	if( !onlineId.isValid() )
	{
		LogMsg( LOG_ERROR, "P2PEngine::fromGuiQueryIdentity invalid id" );
		return false;
	}

	if( getMyOnlineId() == onlineId )
	{
		retNetIdent = *getMyPktAnnounce().getVxNetIdent();
		return true;
	}

	BigListInfo* bigListInfo = m_BigListMgr.findBigListInfo( onlineId );
	if( bigListInfo )
	{
		retNetIdent = *bigListInfo->getVxNetIdent();
		return true;
	}

	return false;
}

//============================================================================
bool P2PEngine::fromGuiSetDefaultUrl( EHostType hostType, std::string& hostUrl )
{
	return getEngineSettings().fromGuiSetDefaultUrl( hostType, hostUrl );
}

//============================================================================
bool P2PEngine::fromGuiQueryHosts( std::string& netHostUrlIn, enum EHostType hostType, std::vector<HostedInfo>& hostedInfoList, VxGUID& hostIdIfNullThenAll )
{
	bool result{ false };
	VxPtopUrl netHostUrl( netHostUrlIn );
	if( netHostUrl.isValid() )
	{
		VxGUID onlineId = netHostUrl.getOnlineId();
		if( getMyOnlineId() == netHostUrl.getOnlineId() )
		{
			return fromGuiQueryMyHostedInfo( hostType, hostedInfoList );
		}

		getHostedListMgr().fromGuiQueryHostedInfoList( hostType, hostedInfoList, hostIdIfNullThenAll );
		return getHostedListMgr().fromGuiQueryHostListFromNetworkHost( netHostUrl, hostType, hostIdIfNullThenAll );
	}

	return result;
}

//============================================================================
bool P2PEngine::fromGuiQueryMyHostedInfo( EHostType hostType, std::vector<HostedInfo>& hostedInfoList )
{
	return getHostedListMgr().fromGuiQueryMyHostedInfo( hostType, hostedInfoList );
}

//============================================================================
bool P2PEngine::fromGuiQueryHostListFromNetworkHost( VxPtopUrl& netHostUrl, EHostType hostType, VxGUID& hostIdIfNullThenAll )
{
	return getHostedListMgr().fromGuiQueryHostListFromNetworkHost( netHostUrl, hostType, hostIdIfNullThenAll );
}

//============================================================================
bool P2PEngine::fromGuiQueryGroupiesFromHosted( VxPtopUrl& hostedUrl, EHostType hostType, VxGUID& onlineIdIfNullThenAll )
{
	return getHostedListMgr().fromGuiQueryGroupiesFromHosted( hostedUrl, hostType, onlineIdIfNullThenAll );
}

//============================================================================
bool P2PEngine::fromGuiDownloadWebPage( enum EWebPageType webPageType, VxGUID& onlineId )
{
	bool result{ false };
	if( eWebPageTypeAboutMe == webPageType )
	{
		PluginBase* plugin = m_PluginMgr.findPlugin( ePluginTypeAboutMePageClient );
		if( plugin )
		{
			result = plugin->fromGuiDownloadWebPage( eWebPageTypeAboutMe, onlineId );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for web page %s", DescribeWebPageType( webPageType ) );
		}
	}
	else if( eWebPageTypeStoryboard == webPageType )
	{
		PluginBase* plugin = m_PluginMgr.findPlugin( ePluginTypeStoryboardClient );
		if( plugin )
		{
			result = plugin->fromGuiDownloadWebPage( eWebPageTypeStoryboard, onlineId );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for web page %s", DescribeWebPageType( webPageType ) );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "Plugin unknown web page type %d", webPageType );
	}

	return result;
}

//============================================================================
bool P2PEngine::fromGuiCancelWebPage( enum EWebPageType webPageType, VxGUID& onlineId )
{
	bool result{ false };
	if( eWebPageTypeAboutMe == webPageType )
	{
		PluginBase* plugin = m_PluginMgr.findPlugin( ePluginTypeAboutMePageClient );
		if( plugin )
		{
			result = plugin->fromGuiCancelWebPage( eWebPageTypeAboutMe, onlineId );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for web page %s", DescribeWebPageType( webPageType ) );
		}
	}
	else if( eWebPageTypeStoryboard == webPageType )
	{
		PluginBase* plugin = m_PluginMgr.findPlugin( ePluginTypeStoryboardClient );
		if( plugin )
		{
			result = plugin->fromGuiCancelWebPage( eWebPageTypeStoryboard, onlineId );
		}
		else
		{
			LogMsg( LOG_ERROR, "Plugin not found for web page %s", DescribeWebPageType( webPageType ) );
		}
	}
	else
	{
		LogMsg( LOG_ERROR, "Plugin unknown web page type %d", webPageType );
	}

	return result;
}

//============================================================================
void P2PEngine::fromGuiUpdatePluginPermission( enum EPluginType pluginType, enum EFriendState pluginPermission )
{
	lockAnnouncePktAccess();
	m_PktAnn.setPluginPermission( pluginType, pluginPermission );
	PktAnnounce* myPktAnn = ( PktAnnounce * )m_PktAnn.makeCopy();
	unlockAnnouncePktAccess();

	LogMsg( LOG_VERBOSE, "P2PEngine::%s Plugin %s permission %s", __func__, DescribePluginType( pluginType ), DescribeFriendState( pluginPermission ) );
	myPktAnn->dumpPermissions( true );

	getPluginMgr().fromGuiUpdatePluginPermission( pluginType, pluginPermission );
	getToGui().toGuiUpdateMyIdent( myPktAnn );
	delete myPktAnn;
	doPktAnnHasChanged( false );
}

//============================================================================
bool P2PEngine::fromGuiDownloadFileList( enum EPluginType pluginType, VxGUID& onlineId, VxGUID& sessionId, uint8_t fileTypes )
{
	bool result{ false };
	PluginBase* plugin = m_PluginMgr.findPlugin( pluginType );
	if( plugin )
	{
		result = plugin->fromGuiDownloadFileList( onlineId, sessionId, fileTypes );
	}
	else
	{
		LogMsg( LOG_ERROR, "Plugin %s fromGuiDownloadFileList failed", DescribePluginType( pluginType ) );
	}

	return result;
}

//============================================================================
bool P2PEngine::fromGuiDownloadFileListCancel( enum EPluginType pluginType, VxGUID& onlineId, VxGUID& sessionId )
{
	bool result{ false };

	PluginBase* plugin = m_PluginMgr.findPlugin( ePluginTypeAboutMePageClient );
	if( plugin )
	{
		result = plugin->fromGuiDownloadFileListCancel( onlineId, sessionId );
	}
	else
	{
		LogMsg( LOG_ERROR, "Plugin %s fromGuiDownloadFileListCancel failed", DescribePluginType( pluginType ) );
	}

	return result;
}

//============================================================================
bool P2PEngine::fromGuiQueryFileHash( FileInfo& fileInfo )
{
	if( fileInfo.getFileLength() && !fileInfo.getFileNameAndPath().empty() )
	{
		bool result = getPluginFileShareServer().fromGuiQueryFileHash( fileInfo );

		if( !result )
		{
			result = getAssetMgr().fromGuiQueryFileHash( fileInfo );
		}

		return result;
	}

	return false;
}

//============================================================================
void P2PEngine::fromGuiFileHashGenerated( std::string& fileName, int64_t fileLen, VxSha1Hash& fileHash )
{
	if( fileLen && !fileName.empty() )
	{
		getPluginFileShareServer().fromGuiFileHashGenerated( fileName, fileLen, fileHash );
		getAssetMgr().fromGuiFileHashGenerated( fileName, fileLen, fileHash );
	}
}

//============================================================================
bool P2PEngine::fromGuiDeleteDatabase( enum EDatabaseType databaseType )
{
	bool result{ false };
    switch( databaseType )
    {

    case eDatabaseTypeAssets:
        return getAssetMgr().getAssetInfoDb().deleteDatabase();
    case eDatabaseTypeBlobAssets:
        return getBlobMgr().getAssetInfoDb().deleteDatabase();
    case eDatabaseTypeConnectMgr:
        return getConnectMgr().getConnectInfoDb().deleteDatabase();
    case eDatabaseTypeEngineParams:
        return getEngineParams().deleteDatabase();
    case eDatabaseTypeEngineSettings:
        return getEngineSettings().deleteDatabase();
    case eDatabaseTypeHostServerJoin:
        return getHostJoinMgr().deleteDatabase();
    case eDatabaseTypeOffers:
        return getOfferMgr().deleteDatabase();
    case eDatabaseTypeThumbs:
        return getThumbMgr().getAssetInfoDb().deleteDatabase();
    case eDatabaseTypeUserJoin:
        return getUserJoinMgr().deleteDatabase();

    case eDatabaseTypeAllUsers:
        return getBigListMgr().deleteDatabase();
    default:
         LogMsg( LOG_ERROR, "P2PEngine::%s Unkonwn Database Type", __func__ );
		 break;
    }

	return result;
}

//============================================================================
void P2PEngine::fromGuiSetIsAutomatedHost( bool automatedHost )
{
	getMyPktAnnounce().setIsAutomatedHost( automatedHost );
}

//============================================================================
bool P2PEngine::fromGuiSendRandConnectSelected( VxGUID& onlineId, bool isSelected )
{
	VxGUID sessionId;
	return fromGuiSendRandConnectAction( onlineId,
										 isSelected ? eRandActionSelectUser : eRandActionDeselectUser,
										 sessionId,
										 GetTimeStampMs(),
										 eOfferTypeUnknown );
}

//============================================================================
bool P2PEngine::fromGuiSendRandConnectAction( VxGUID& onlineId,
											  enum ERandAction randAction,
											  VxGUID sessionId,
                                              uint64_t timeRequestedMs,
                                              EOfferType offerType )
{
	bool result{ false };

	std::shared_ptr<VxSktBase> sktBase = getConnectIdListMgr().findAnyHostConnection( eHostTypeRandomConnect );
	if( sktBase && sktBase->isConnected() && sktBase->getIsPeerPktAnnSet() )
	{
		PktRandConnectReq pktReq;
		pktReq.setPluginNum( ePluginTypeHostRandomConnect );
        size_t pktSize = pktReq.getPktLength();
        if(pktSize & 0x0f)
        {
            LogMsg( LOG_ERROR, "PktRandConnectReq size %d is invalid", pktSize );
        }
		GroupieId groupieId( getMyOnlineId(), sktBase->getPeerOnlineId(), eHostTypeRandomConnect );

		if( !sessionId.isValid() )
		{
			sessionId.initializeWithNewVxGUID();
		}

		if( !timeRequestedMs )
		{
			timeRequestedMs = GetTimeStampMs();
		}

		pktReq.setGroupieId( groupieId );
		pktReq.setToUserOnlineId( onlineId );
		pktReq.setRandAction( randAction );
		pktReq.setSessionId( sessionId );
		pktReq.setTimeRequestedMs( timeRequestedMs );
		pktReq.setOfferType( offerType );
        pktReq.setSrcOnlineId( getMyOnlineId() );
		pktReq.setDestOnlineId( sktBase->getPeerOnlineId() );
		result = 0 == sktBase->txPacketWithDestId( &pktReq );
	}
	else
	{
		LogMsg( LOG_ERROR, "P2PEngine::%s No Connection", __func__ );
	}

	return result;
}

//============================================================================
void P2PEngine::fromGuiApplyNetHostSettings( NetHostSetting& netHostSetting )
{
	static bool firstSet{ true };
	if( firstSet )
	{
		// clear any previous determined external ip and detect ip again
		firstSet = false;
		lockAnnouncePktAccess();
		m_PktAnn.clearOnlineIpAddress();
		unlockAnnouncePktAccess();
	}

	getPeerMgr().setUpnpEnable( netHostSetting.getUseUpnpPortForward() );
	getPeerMgr().setUseIpv6( netHostSetting.getUseIpv6() );

    NetHostSetting origSettings;
    m_EngineSettings.getNetHostSettings( origSettings );
	bool settingsHaveChange = origSettings != netHostSetting;
	if( settingsHaveChange )
	{
		std::string networkKey = netHostSetting.getNetworkKey();
		if( networkKey.empty() )
		{
			LogMsg( LOG_FATAL, "P2PEngine::%s empty network key", __func__ );
			vx_assert( false );
		}

		getNetStatusAccum().setNetworkKey( networkKey );
		getNetStatusAccum().setNetworkHostUrl( netHostSetting.getNetworkHostUrl() );
		getNetStatusAccum().setConnectionTestHostUrl( netHostSetting.getConnectTestUrl() );

		m_EngineSettings.setNetHostSettings( netHostSetting );
		// TODO remove NetworkStateMachine
		m_NetworkMgr.updateFromEngineSettings( m_EngineSettings );
	}

	getNetStatusAccum().setFirewallTestType( (EFirewallTestType)netHostSetting.getFirewallTestType() );
	// so listen thread gets a head start
	getNetStatusAccum().setUseIpv6( netHostSetting.getUseIpv6(), netHostSetting.getTcpPort() );

	bool haveFixedIp{ false };
		
	m_NetServicesMgr.addNetActionToQueue( eNetActionWaitForInternet );

    if( eFirewallTestAssumeNoFirewall == netHostSetting.getFirewallTestType() && !netHostSetting.getUserSpecifiedExternIpAddr().empty() )
    {
		std::string externIp = netHostSetting.getUserSpecifiedExternIpAddr();
		if( !externIp.empty() )
		{
			InetAddress inetAddr;
			inetAddr.setIp( externIp.c_str() );
			std::string netIp = inetAddr.toString();
			if( netIp != externIp )
			{
				LogMsg( LOG_ERROR, "IP %s does not match %s ", netIp.c_str(), externIp.c_str() );
			}

			updateMyPktAnnIpAddress( externIp );
			getNetStatusAccum().setUseFixedIp( externIp );
			haveFixedIp = true;
		}
    }

	if( !haveFixedIp )
	{
		m_NetServicesMgr.addNetActionToQueue( eNetActionResolveConnectTestUrl );
		m_NetServicesMgr.addNetActionToQueue( eNetActionIsPortOpen );
	}

	m_NetServicesMgr.addNetActionToQueue( eNetActionResolveNetworkHostUrl );
	m_NetServicesMgr.addNetActionToQueue( eNetActionResolveDefaultUserHosts );
}

//============================================================================
bool P2PEngine::fromGuiQueryFriendRequest( std::vector<std::shared_ptr<FriendRequestInfo>>& friendRequestList, VxGUID& onlineIdIfNullThenAll )
{
	return getFriendRequestMgr().fromGuiQueryFriendRequest( friendRequestList, onlineIdIfNullThenAll );
}

//============================================================================
bool P2PEngine::fromGuiSendFriendRequest( VxGUID& onlineId, std::string& requestText, EFriendState myFriendshipToHim )
{
	PluginBase* pluginBase = m_PluginMgr.getPlugin( ePluginTypeFriendRequest );
	if( pluginBase )
	{
		return ((PluginFriendRequest*)pluginBase)->fromGuiSendFriendRequest( onlineId, requestText, myFriendshipToHim );
	}

	return false;
}

//============================================================================
void P2PEngine::fromGuiBlockUser( VxGUID& onlineId, bool fromThread )
{
	if( fromThread )
	{
		VxNetIdent* netIdent = getBigListMgr().findNetIdent( onlineId );
		if( !netIdent )
		{
			LogMsg( LOG_ERROR, "P2PEngine::%s user not found %s", __func__, onlineId.toOnlineIdString().c_str() );
			return;
		}

		fromGuiChangeMyFriendshipToHim( onlineId, eFriendStateIgnore, netIdent->getHisFriendshipToMe() );
	}
	else
	{
		if( LogEnabled( eLogUsers ) )LogMsg( LOG_VERBOSE, "P2PEngine::%s queued %s", __func__, onlineId.toOnlineIdString().c_str() );
		m_FromGuiMgr.fromGuiBlockUser( onlineId );
	}
}

//============================================================================
void P2PEngine::fromGuiScanFolderForMedia( VxGUID& appInstId, std::string dirToScan, uint8_t fileTypeFilter, bool fromThread )
{
	if( fromThread )
	{
		m_FileMgr.fromGuiScanFolderForMedia( *this, appInstId, dirToScan, fileTypeFilter );
	}
	else
	{
		m_FromGuiMgr.fromGuiScanFolderForMedia( appInstId, dirToScan, fileTypeFilter );
	}
}

//============================================================================
void P2PEngine::fromGuiScanItemReceived( VxGUID& appInstId )
{
	m_FileMgr.fromGuiScanItemReceived( appInstId );
}

//============================================================================
void P2PEngine::fromGuiScanFolderCancel( VxGUID& appInstId )
{
	m_FileMgr.fromGuiScanFolderCancel( appInstId );
}
