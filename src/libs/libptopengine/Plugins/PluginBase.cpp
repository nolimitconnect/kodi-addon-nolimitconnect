//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#include "PluginBase.h"

#include "P2PSession.h"
#include "PluginMgr.h"
#include "RxSession.h"
#include "TxSession.h"

#include <GuiInterface/IDefs.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxPtopUrl.h>
#include <CoreLib/VxTime.h>

#include <PktLib/PktsHostInvite.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/PktsSession.h>

#include <NetLib/VxSktBase.h>

namespace
{
    bool ShouldUseSessionScopedAccessElevation( EPluginType pluginType )
    {
        switch( pluginType )
        {
        case ePluginTypeMessenger:
        case ePluginTypePersonFileXfer:
        case ePluginTypeVideoChat:
        case ePluginTypeVoicePhone:
        case ePluginTypeTruthOrDare:
            return true;
        default:
            return false;
        }
    }
}

VxMutex	PluginBase::m_VoicePairTxMutex;
std::vector<std::pair<EPluginType, VxGUID>>	PluginBase::m_VoiceTxList;
VxMutex	PluginBase::m_VoicePairRxMutex;
std::vector<std::pair<EPluginType, VxGUID>>	PluginBase::m_VoiceRxList;

VxMutex	PluginBase::m_VideoPairTxMutex;
std::vector<std::pair<EPluginType, VxGUID>>	PluginBase::m_VideoTxList;
VxMutex	PluginBase::m_VideoPairRxMutex;
std::vector<std::pair<EPluginType, VxGUID>>	PluginBase::m_VideoRxList;

//============================================================================
bool PluginBase::isVoicePlugin( void ) 
{
    return m_ePluginType == ePluginTypeCamServer ||
        m_ePluginType == ePluginTypeCamClient ||
        m_ePluginType == ePluginTypeTruthOrDare ||
        m_ePluginType == ePluginTypeVideoChat ||
        m_ePluginType == ePluginTypeVoicePhone ||
        m_ePluginType == ePluginTypePushToTalk;
}

//============================================================================
std::string PluginBase::getThumbXferDbName( EPluginType pluginType )
{
    std::string thumbXferName( "ThumbXferDb" );
    thumbXferName += std::to_string( (int)pluginType );
    return thumbXferName + ".db3";
}

//============================================================================
std::string PluginBase::getThumbXferThreadName( EPluginType pluginType )
{
    std::string thumbXferName( "ThumbXferDb" );
    thumbXferName += std::to_string( (int)pluginType );
    return thumbXferName + ".db3";
}

//============================================================================
PluginBase::PluginBase( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PktPluginHandlerBase()
, m_ePluginType( pluginType )
, m_Engine( engine )
, m_PluginMgr( pluginMgr )
, m_MyIdent( myIdent )
, m_PluginMutex()
, m_AssetMgr( engine.getAssetMgr() )
, m_ThumbMgr( engine.getThumbMgr() )
, m_ThumbXferInterface( *this )
, m_ThumbXferMgr( engine, engine.getThumbMgr(), m_ThumbXferInterface )
{
    m_MediaSessionId.initializeWithNewVxGUID();
    m_PluginSetting.setPluginType( pluginType );
}

//============================================================================
IToGui& PluginBase::getToGui()
{ 
    return m_Engine.getToGui(); 
}

//============================================================================
void PluginBase::setPluginType( EPluginType pluginType )
{
    m_ePluginType = pluginType;
    m_PluginSetting.setPluginType( pluginType );
}

//============================================================================
void PluginBase::pluginStartup( void )
{
    if( getPluginType() != ePluginTypeInvalid )
    {
        m_Engine.getPluginSetting( getPluginType(), m_PluginSetting );
    }
}

//============================================================================
const char* PluginBase::describePlugin( void )
{
    return DescribePluginType( getPluginType() );
}

//============================================================================
bool PluginBase::setPluginSetting( PluginSetting& pluginSetting, int64_t modifiedTimeMs )
{
    m_PluginSetting = pluginSetting;
    onPluginSettingChange( m_PluginSetting, modifiedTimeMs );

    return true;
}

//============================================================================
EHostType PluginBase::getHostType( void )
{
    EHostType hostType = eHostTypeUnknown;
    switch( getPluginType() )
    {
    case ePluginTypeClientChatRoom:
    case ePluginTypeHostChatRoom:
        hostType = eHostTypeChatRoom;
        break;

    case ePluginTypeClientConnectTest:
    case ePluginTypeHostConnectTest:
        hostType = eHostTypeConnectTest;
        break;

    case ePluginTypeClientGroup:
    case ePluginTypeHostGroup:
        hostType = eHostTypeGroup;
        break;

    case ePluginTypeClientRandomConnect:
    case ePluginTypeHostRandomConnect:
        hostType = eHostTypeRandomConnect;
        break;

    case ePluginTypeClientNetwork:
    case ePluginTypeHostNetwork:
        hostType = eHostTypeNetwork;
        break;

    default:
        break;
    }

    return hostType;
}

//============================================================================
EPluginType	PluginBase::getClientPluginType( void )
{
    EPluginType clientPluginType = getPluginType();
    switch( clientPluginType )
    {

    case ePluginTypeAboutMePageServer:
        clientPluginType = ePluginTypeAboutMePageClient;
        break;

    case ePluginTypeCamServer:
        clientPluginType = ePluginTypeCamClient;
        break;

    case ePluginTypeFileShareServer:
        clientPluginType = ePluginTypeFileShareClient;
        break;

    case ePluginTypeHostChatRoom:
        clientPluginType = ePluginTypeClientChatRoom;
        break;

    case ePluginTypeHostConnectTest:
        clientPluginType = ePluginTypeClientConnectTest;
        break;

    case ePluginTypeHostGroup:
        clientPluginType = ePluginTypeClientGroup;
        break;

    case ePluginTypeHostRandomConnect:
        clientPluginType = ePluginTypeClientRandomConnect;
        break;

    case ePluginTypeStoryboardServer:
        clientPluginType = ePluginTypeStoryboardClient;
        break;

    default:
        break;
    }

    return clientPluginType;
}

//============================================================================
EPluginType	PluginBase::getServerPluginType( void )
{
    EPluginType serverPluginType = getPluginType();
    switch( serverPluginType )
    {

    case ePluginTypeAboutMePageClient:
        serverPluginType = ePluginTypeAboutMePageServer;
        break;

    case ePluginTypeCamClient:
        serverPluginType = ePluginTypeCamServer;
        break;

    case ePluginTypeClientChatRoom:
        serverPluginType = ePluginTypeHostChatRoom;
        break;

    case ePluginTypeClientConnectTest:
        serverPluginType = ePluginTypeHostConnectTest;
        break;

    case ePluginTypeClientGroup:
        serverPluginType = ePluginTypeHostGroup;
        break;

    case ePluginTypeClientRandomConnect:
        serverPluginType = ePluginTypeHostRandomConnect;
        break;

    case ePluginTypeFileShareClient:
        serverPluginType = ePluginTypeFileShareServer;
        break;

    case ePluginTypeStoryboardClient:
        serverPluginType = ePluginTypeStoryboardServer;
        break;

    default:
        break;
    }

    return serverPluginType;
}

//============================================================================
EAppState PluginBase::getPluginState( void )
{
	if( eFriendStateIgnore == getPluginPermission() )
	{
		return eAppStatePermissionErr;
	}

	return m_ePluginState;
}

//============================================================================
EFriendState PluginBase::getPluginPermission( void )
{ 
	return m_Engine.getMyPktAnnounce().getPluginPermission( m_ePluginType ); 
}

//============================================================================
void PluginBase::setPluginPermission( EFriendState eFriendState )		
{ 
	EFriendState prevState = m_Engine.getMyPktAnnounce().getPluginPermission( m_ePluginType ); 
	if( prevState != eFriendState )
	{
        m_Engine.lockAnnouncePktAccess();
		m_Engine.getMyPktAnnounce().setPluginPermission( m_ePluginType, eFriendState );
        m_Engine.unlockAnnouncePktAccess();
		m_Engine.doPktAnnHasChanged( false );
	}
};

//============================================================================
bool PluginBase::isAccessAllowed( VxNetIdent* netIdent, bool logAccessError, const char* accessReason )
{
    if( IsClientPluginType( getPluginType() ) )
    {
        // clients make requests
        return true;
    }

	EFriendState curPermission = m_Engine.getMyPktAnnounce().getPluginPermission( m_ePluginType ); 
    if( eFriendStateIgnore != curPermission && eFriendStateIgnore != netIdent->getMyFriendshipToHim() )
    {
        if( netIdent->getMyFriendshipToHim() >= curPermission || netIdent->getMyOnlineId() == m_Engine.getMyOnlineId() )
        {
            return true;
        }

        // For host service plugins, members of the same hosted service get guest-level access
        if( IsHostPluginType( getPluginType() ) && m_Engine.isMemberGuest( getPluginType(), netIdent->getMyOnlineId() ) &&
            eFriendStateGuest >= curPermission )
        {
            // fellow member has guest-level access to host service
            return true;
        }
    }

    // For offer/session plugins only, keep access open when we already have a session
    // for this peer (temporary session-scoped elevation).
    if( ShouldUseSessionScopedAccessElevation( getPluginType() ) )
    {
        VxGUID onlineId = netIdent->getMyOnlineId();
        if( fromGuiIsPluginInSession( onlineId ) )
        {
            return true;
        }
    }

    if( logAccessError )
    {
        if( accessReason )
        {
            LogMsg( LOG_WARN, "%s %s Access Not Allowed to %s %s %s", DescribePluginType( getPluginType() ), accessReason,
                netIdent->getMyOnlineId().describeVxGUID().c_str(), DescribeFriendState( netIdent->getMyFriendshipToHim() ), netIdent->getOnlineName() );
        }
        else
        {
            LogMsg( LOG_WARN, "%s Access Not Allowed to %s %s %s", DescribePluginType( getPluginType() ),
                netIdent->getMyOnlineId().describeVxGUID().c_str(), DescribeFriendState( netIdent->getMyFriendshipToHim() ), netIdent->getOnlineName() );
        }
    }

	return false;
}

//============================================================================
bool PluginBase::isPluginEnabled( void )
{
	return ( eFriendStateIgnore != getPluginPermission() );
}

//============================================================================
void PluginBase::onSessionStart( PluginSessionBase* poSession, bool pluginIsLocked )
{
	m_Engine.onSessionStart( poSession->getPluginType(), poSession->getSendToId() );
}

//============================================================================
bool PluginBase::txPacket( VxGUID onlineId, std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* poPkt, EPluginType overridePlugin )
{
    if( nullptr == sktBase )
    {
        LogMsg( LOG_WARN, "PluginBase::txPacket NULL sktBase" );
        return false;
    }

    return m_PluginMgr.pluginApiTxPacket( m_ePluginType, onlineId, sktBase, poPkt, overridePlugin );
}

//============================================================================
void PluginBase::fromGuiGetFileShareSettings( FileShareSettings& fileShareSettings )
{
}

//============================================================================
void PluginBase::fromGuiSetFileShareSettings( FileShareSettings& fileShareSettings )
{
}

//============================================================================
EXferError PluginBase::fromGuiFileXferControl( VxGUID& onlineId, EXferAction xferAction, FileInfo& fileInfo )
{
	return eXferErrorBadParam;
}

//============================================================================ 
bool PluginBase::fromGuiInstMsg( VxGUID& onlineId, const char* pMsg )
{
	return false;
}

//============================================================================ 
bool PluginBase::fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk )
{
    return false;
}

//============================================================================ 
void PluginBase::makeShortFileName( const char* pFullFileName, std::string& strShortFileName )
{
    VxFileUtil::makeShortFileName( pFullFileName, strShortFileName );
}

//============================================================================ 
EPluginAccess PluginBase::canAcceptNewSession( VxNetIdent* netIdent ) 
{ 
    return netIdent->getHisAccessPermissionFromMe( m_ePluginType ); 
}

//============================================================================ 
P2PSession* PluginBase::createP2PSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginBase::%s no session user %s", __func__,
        m_Engine.describeUser( onlineId ).c_str() );
    return new P2PSession( sktBase, onlineId, m_ePluginType );
}

//============================================================================ 
P2PSession* PluginBase::createP2PSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginBase::%s lclSession %s user %s", __func__,
        lclSessionId.toHexString().c_str(),
        m_Engine.describeUser( onlineId ).c_str() );
    return new P2PSession( lclSessionId, sktBase, onlineId, m_ePluginType );
}

//============================================================================ 
RxSession *	PluginBase::createRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    return new RxSession( sktBase, onlineId, m_ePluginType );
}

//============================================================================ 
RxSession *	PluginBase::createRxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    return  new RxSession( lclSessionId, sktBase, onlineId, m_ePluginType );
}

//============================================================================ 
TxSession *	PluginBase::createTxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    return new TxSession( sktBase, onlineId, m_ePluginType );
}

//============================================================================ 
TxSession *	PluginBase::createTxSession( VxGUID& lclSessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
    return new TxSession( lclSessionId, sktBase, onlineId, m_ePluginType );
}

//============================================================================ 
void PluginBase::onAppStartup( void )
{
}

//============================================================================ 
void PluginBase::onAppShutdown( void )
{
}

//============================================================================ 
EPluginType PluginBase::getDestinationPluginOverride( EHostType hostType )
{
    if( eHostTypeNetwork == hostType )
    {
        // no matter which plugin we send from if the destination host is network host
        // the always set the packet plugin destination to network host
        if( ePluginTypeHostNetwork == m_ePluginType )
        {
            LogMsg( LOG_ERROR, "Tried to send to host %s from plugin %s", DescribeHostType( hostType ), DescribePluginType( m_ePluginType ) );
            vx_assert( false );
            return ePluginTypeInvalid;
        }

        return ePluginTypeHostNetwork;
    }

    if( eHostTypePeerUser == hostType )
    {
        // directly connected peer users can only access peer type plugins (Not Client/Host)
        return ePluginTypeInvalid;
    }

    if( eHostTypeGroup == hostType )
    {
        switch( m_ePluginType )
        {
        case ePluginTypeClientGroup:
            return ePluginTypeHostGroup;
        case ePluginTypeHostGroup:
            return ePluginTypeClientGroup;
        default:
            LogMsg( LOG_ERROR, "Tried to send to host %s from plugin %s", DescribeHostType( hostType ), DescribePluginType( m_ePluginType ) );
            vx_assert( false );
            return ePluginTypeInvalid;
        }
    }

    if( eHostTypeChatRoom == hostType )
    {
        switch( m_ePluginType )
        {
        case ePluginTypeClientChatRoom:
            return ePluginTypeHostChatRoom;
        case ePluginTypeHostChatRoom:
            return ePluginTypeClientChatRoom;
        default:
            LogMsg( LOG_ERROR, "Tried to send to host %s from plugin %s", DescribeHostType( hostType ), DescribePluginType( m_ePluginType ) );
            vx_assert( false );
            return ePluginTypeInvalid;
        }
    }

    if( eHostTypeRandomConnect == hostType )
    {
        switch( m_ePluginType )
        {
        case ePluginTypeClientRandomConnect:
            return ePluginTypeHostRandomConnect;
        case ePluginTypeHostRandomConnect:
            return ePluginTypeClientRandomConnect;
        default:
            LogMsg( LOG_ERROR, "Tried to send to host %s from plugin %s", DescribeHostType( hostType ), DescribePluginType( m_ePluginType ) );
            vx_assert( false );
            return ePluginTypeInvalid;
        }
    }

    if( eHostTypeConnectTest == hostType )
    {
        switch( m_ePluginType )
        {
        case ePluginTypeClientConnectTest:
            return ePluginTypeHostConnectTest;
        case ePluginTypeHostConnectTest:
            return ePluginTypeClientConnectTest;
        default:
            LogMsg( LOG_ERROR, "Tried to send to host %s from plugin %s", DescribeHostType( hostType ), DescribePluginType( m_ePluginType ) );
            vx_assert( false );
            return ePluginTypeInvalid;
        }
    }

    LogMsg( LOG_ERROR, "Tried to send to Unknown Host Type %d from plugin %s", hostType, DescribePluginType( m_ePluginType ) );
    vx_assert( false );
    return ePluginTypeInvalid;
}

//============================================================================
EPluginAccess PluginBase::getPluginAccessState( VxNetIdent* netIdent )
{
    EPluginAccess pluginAccess = ePluginAccessNotSet;

    EFriendState pluginState = m_MyIdent->getPluginPermission( getPluginType() );
    if( eFriendStateIgnore == pluginState )
    {
        // we are not enabled
        pluginAccess = ePluginAccessDisabled;
    }
    else
    {
        if( netIdent->isIgnored() )
        {
            pluginAccess = ePluginAccessIgnored;
        }
        else
        {
            EFriendState friendState = netIdent->getMyFriendshipToHim();
            // everybody gets at least guest permission
            if( m_Engine.getConnectIdListMgr().isHosted( netIdent->getMyOnlineId() ) && friendState == eFriendStateAnonymous )
            {
                friendState = eFriendStateGuest;
            }

            if( friendState < pluginState )
            {
                // not enough permission
                pluginAccess = ePluginAccessLocked;
            }
            else
            {
                pluginAccess = ePluginAccessOk;
            }
        }
    }

    return pluginAccess;
}

//============================================================================
void PluginBase::logCommError( ECommErr commErr, const char* desc, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    LogMsg( LOG_ERROR, "%s %s from %s %s", desc, DescribeCommError( commErr ), netIdent->getOnlineName(), sktBase->describeSktConnection().c_str() );
}

//============================================================================
ECommErr PluginBase::getCommAccessState( VxNetIdent* netIdent )
{
    ECommErr commErr{ eCommErrNone };
    if( !isPluginEnabled() )
    {
        commErr = eCommErrPluginNotEnabled;
    }
    else
    {
        EPluginAccess pluginAccess = getPluginAccessState( netIdent );
        if( ePluginAccessOk != pluginAccess )
        {
            commErr = eCommErrPluginPermission;
        }
    }

    return commErr;
}

//============================================================================
void PluginBase::toGuiFileUploadStart( VxGUID& onlineId, VxGUID& lclSessionId, FileInfo& fileInfo )
{
    m_Engine.getToGui().toGuiFileUploadStart( onlineId, getPluginType(), lclSessionId, fileInfo );
}

//============================================================================
void PluginBase::toGuiFileDownloadStart( VxGUID& onlineId, VxGUID& lclSessionId, FileInfo& fileInfo )
{
    m_Engine.getToGui().toGuiFileDownloadStart( onlineId, getPluginType(), lclSessionId, fileInfo );
}

//============================================================================
void PluginBase::toGuiFileXferState( VxGUID& lclSessionId, EXferDirection xferDir, EXferState xferState, EXferError xferErr, int param )
{
    m_Engine.getToGui().toGuiFileXferState( getPluginType(), lclSessionId, xferDir, xferState, xferErr, param );
}

//============================================================================
void PluginBase::toGuiFileDownloadComplete( VxGUID& lclSessionId, std::string& fileName, EXferError xferError )
{
    m_Engine.getToGui().toGuiFileDownloadComplete( getPluginType(), lclSessionId, fileName, xferError );
}

//============================================================================
void PluginBase::toGuiFileUploadComplete( VxGUID& lclSessionId, std::string& fileName, EXferError xferError )
{
    m_Engine.getToGui().toGuiFileUploadComplete( getPluginType(), lclSessionId, fileName, xferError );
}

//============================================================================
void PluginBase::handleToGuiOfferRequest( VxNetIdent* netIdent, PktPluginOfferReq* pktReq )
{
    OfferBaseInfo offerInfo;
    if( offerInfo.extractFromBlob( pktReq->getBlobEntry() ) )
    {
        IToGui::getIToGui().toGuiRxedPluginOffer( netIdent->getMyOnlineId(), offerInfo);
    }
}

//============================================================================
void PluginBase::handleToGuiOfferResponse( VxNetIdent* netIdent, PktPluginOfferReply* pktReply )
{
    OfferBaseInfo offerInfo;
    if( offerInfo.extractFromBlob( pktReply->getBlobEntry() ) )
    {
        IToGui::getIToGui().toGuiRxedOfferReply( netIdent->getMyOnlineId(), offerInfo );
    }
}

//============================================================================
void PluginBase::handlePktHostInviteSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostInviteSearchReply* pktReply = (PktHostInviteSearchReply*)pktHdr;
    if( pktReply->getCommError() )
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "PluginBase::onPktHostInviteSearchReply comm err %s", DescribeCommError( pktReply->getCommError() ) );
        logCommError( pktReply->getCommError(), "PktHostInviteSearchReply", sktBase, netIdent );
        m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getCommError() );
    }
    else
    {
        updateFromHostInviteSearchBlob( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getBlobEntry(), pktReply->getInviteCountThisPkt() );

        if( pktReply->getMoreInvitesExist() )
        {
            if( !requestMoreHostInvitesFromNetworkHost( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getNextSearchOnlineId() ) )
            {
                m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
            }
        }
        else
        {
            m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
        }
    }
}

//============================================================================
void PluginBase::handlePktHostInviteMoreReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
    PktHostInviteMoreReply* pktReply = (PktHostInviteMoreReply*)pktHdr;
    if( pktReply->getCommError() )
    {
        logCommError( pktReply->getCommError(), "PktHostInviteSearchReply", sktBase, netIdent );
        m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getCommError() );
    }
    else
    {
        LogModule( eLogHostSearch, LOG_DEBUG, "PluginBase::onPktHostInviteMoreReply %d hosts", pktReply->getInviteCountThisPkt() );

        updateFromHostInviteSearchBlob( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getBlobEntry(), pktReply->getInviteCountThisPkt() );
        if( pktReply->getMoreInvitesExist() )
        {
            if( !requestMoreHostInvitesFromNetworkHost( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, pktReply->getNextSearchOnlineId() ) )
            {
                m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
            }
        }
        else
        {
            m_Engine.getHostedListMgr().hostSearchCompleted( pktReply->getHostType(), pktReply->getSearchSessionId(), sktBase, netIdent, eCommErrNone );
        }
    }
}

//============================================================================
void PluginBase::updateFromHostInviteSearchBlob( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, PktBlobEntry& blobEntry, int hostInfoCount )
{
    LogModule( eLogHostSearch, LOG_DEBUG, "PluginBase::updateFromHostInviteSearchBlob rxed %d hosts", hostInfoCount );
    blobEntry.resetRead();
    for( int i = 0; i < hostInfoCount; i++ )
    {
        HostedInfo hostedInfo;
        if( hostedInfo.extractFromSearchBlob( blobEntry ) )
        {
            if( eHostTypeUnknown == hostedInfo.getHostType() && eHostTypeUnknown != hostType )
            {
                hostedInfo.setHostType( hostType );
            }

            if( netIdent->getMyOnlineId() == hostedInfo.getAdminOnlineId() )
            {
                // handle the case where network host is also hosting a group
                // normally the identity is never sent to the gui because this is a temporary connection
                // but gui needs the identity to show host list
                if( sktBase->isTempConnection() )
                {
                    m_Engine.getToGui().toGuiContactAnythingChange( netIdent );
                }
            }
            else
            {
                assureIdentityExist( hostedInfo );
            }

            m_Engine.getHostedListMgr().hostSearchResult( hostType, searchSessionId, sktBase, netIdent, hostedInfo );
        }
        else
        {
            LogMsg( LOG_ERROR, "Could not extract HostInviteSearchBlob" );
            break;
        }
    }
}

//============================================================================
bool PluginBase::requestMoreHostInvitesFromNetworkHost( EHostType hostType, VxGUID& searchSessionId, std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& nextHostOnlineId )
{
    LogModule( eLogHostSearch, LOG_DEBUG, "PluginBase::requestMoreHostInvitesFromNetworkHost" );
    PktHostInviteMoreReq pktReq;
    pktReq.setHostType( hostType );
    pktReq.setSearchSessionId( searchSessionId );
    pktReq.setNextSearchOnlineId( nextHostOnlineId );
    pktReq.setPluginNum( (uint8_t)ePluginTypeHostNetwork );
    return txPacket( netIdent->getMyOnlineId(), sktBase, &pktReq, ePluginTypeHostNetwork );
}

//============================================================================
EPluginType PluginBase::getAssetOverridePluginType( void )
{
    // Asset transfer packets must target the remote counterpart plugin.
    // For ChatRoom: client sends to host (host relays to members), host sends to client.
    // For Group/RandomConnect: members send directly to each other's CLIENT plugins,
    //   host admin sends to client plugins.
    EPluginType pluginType = getPluginType();
    
    if( IsClientPluginType( pluginType ) )
    {
        // Group and RandomConnect clients send to other members' CLIENT plugins (peer-to-peer)
        // ChatRoom clients send to HOST plugin (host admin relays)
        if( pluginType == ePluginTypeClientGroup || pluginType == ePluginTypeClientRandomConnect )
        {
            // Return same client type so members communicate via their client plugins
            return pluginType;
        }
        return ClientPluginToHostPluginType( pluginType );
    }

    // Host plugins always send to client plugins
    return HostPluginToClientPluginType( pluginType );
}

//============================================================================
bool PluginBase::assureIdentityExist( HostedInfo& hostedInfo )
{
    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( hostedInfo.getAdminOnlineId() );
    if( netIdent )
    {
        return true;
    }

    // for the case of host listing but user has not connected to host and does not have any identity info
    // create a temporary identity with minimal permissions for gui to use until pkt announce recieved
    bool hasIdent{ false };
    if( hostedInfo.getAdminOnlineId().isValid() )
    {
        // create a temporary identity until a connection is made
        VxNetIdent tempIdent;

        bool ipv6{ false };
        std::string url = hostedInfo.getHostInviteUrl();
        VxPtopUrl ptopUrl( url );
        if( ptopUrl.isValid() )
        {
            VxGUID ptopOnlineId = ptopUrl.getOnlineId();
            if( ptopOnlineId != hostedInfo.getAdminOnlineId() )
            {
                LogMsg( LOG_ERROR, "PluginBase::%s admin id %s does not match url id %s", __func__,
                        hostedInfo.getAdminOnlineId().toOnlineIdString().c_str(),
                        ptopOnlineId.toOnlineIdString().c_str() );
                return false;
            }

            tempIdent.setOnlinePort( ptopUrl.getPort() );
            tempIdent.setOnlineIpAddress( ptopUrl.getHost() );
            tempIdent.setMyOnlineId( ptopUrl.getOnlineId() );

            tempIdent.setOnlineName( hostedInfo.getHostTitle().c_str());
            tempIdent.setOnlineDescription( hostedInfo.getHostDescription().c_str() );
            tempIdent.setMyFriendshipToHim( eFriendStateAnonymous );
            tempIdent.setHisFriendshipToMe( eFriendStateAnonymous );
            tempIdent.setAvatarGuid( hostedInfo.getThumbId(), GetGmtTimeMs() );
            if( IsHostARelayForUsers( ptopUrl.getHostType() ) )
            {
                tempIdent.setPluginPermission( HostTypeToHostPlugin( ptopUrl.getHostType() ), eFriendStateAnonymous );
                m_Engine.getToGui().toGuiContactAnythingChange( &tempIdent );
                hasIdent = true;

                vx_assert( tempIdent.isValidNetIdent() );
            }
        }
    }

    if( !hasIdent )
    {
        LogMsg( LOG_ERROR, "PluginBase::%s could not create host temporary identity" );
    }

    return hasIdent;
}

//============================================================================
bool PluginBase::addVoicePairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VoicePairTxMutex.lock();
    for( auto& pair : m_VoiceTxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            found = true;
            break;
        }
    }

    if( !found )
    {
        m_VoiceTxList.emplace_back( std::make_pair( pluginType, onlineId ) );
    }

    m_VoicePairTxMutex.unlock();
    if( found && LogEnabled( eLogVoice ) )
    {
        LogModule( eLogVoice, LOG_ERROR, "PluginBase::%s online id %s already exists", __func__, onlineId.toHexString().c_str() );
    }

    return !found;
}

//============================================================================
bool PluginBase::removeVoicePairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VoicePairTxMutex.lock();
    for( auto iter = m_VoiceTxList.begin(); iter != m_VoiceTxList.end(); ++iter )
    {
        if( pluginType == iter->first && onlineId == iter->second )
        {
            found = true;
            m_VoiceTxList.erase( iter );
            break;
        }
    }

    m_VoicePairTxMutex.unlock();
    if( !found && LogEnabled( eLogVoice ) )
    {
        LogModule( eLogVoice, LOG_ERROR, "PluginBase::%s online id %s not found %s", __func__, onlineId.toHexString().c_str(), DescribePluginType( pluginType ) );
    }

    return found;
}

//============================================================================
bool PluginBase::userNeedsVoicePairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool needsTx{ false };
    m_VoicePairTxMutex.lock();
    for( auto& pair : m_VoiceTxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            needsTx = true;
            break;
        }
    }

    m_VoicePairTxMutex.unlock();
    return needsTx;
}

//============================================================================
int PluginBase::needVoiceTxCount( EPluginType pluginType )
{
    int needCnt{ 0 };
    m_VoicePairTxMutex.lock();
    for( auto& pair : m_VoiceTxList )
    {
        if( pair.first == pluginType )
        {
            needCnt++;
        }
    }

    m_VoicePairTxMutex.unlock();
    return needCnt;
}

//============================================================================
bool PluginBase::getVoiceTxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList )
{
    onlineIdList.clear();
    m_VoicePairTxMutex.lock();
    for( auto& pair : m_VoiceTxList )
    {
        if( pair.first == pluginType )
        {
            onlineIdList.emplace_back( pair.second );
        }
    }

    m_VoicePairTxMutex.unlock();
    return !onlineIdList.empty();
}

//============================================================================
bool PluginBase::isFirstVoicePairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool isFirst{ false };
    m_VoicePairTxMutex.lock();
    for( auto& pair : m_VoiceTxList )
    {
        if( pair.second == onlineId )
        {
            if( pair.first == pluginType )
            {
                isFirst = true;
            }

            break;
        }
    }

    m_VoicePairTxMutex.unlock();
    return isFirst;
}

//============================================================================
bool PluginBase::addVoicePairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VoicePairRxMutex.lock();
    for( auto& pair : m_VoiceRxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            found = true;
            break;
        }
    }

    if( !found )
    {
        m_VoiceRxList.emplace_back( std::make_pair( pluginType, onlineId ) );
    }

    m_VoicePairRxMutex.unlock();
    if( found && LogEnabled( eLogVoice ) )
    {
        LogModule( eLogVoice, LOG_ERROR, "PluginBase::%s online id %s already exists", __func__, onlineId.toHexString().c_str() );
    }

    return !found;

}

//============================================================================
bool PluginBase::removeVoicePairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VoicePairRxMutex.lock();
    for( auto iter = m_VoiceRxList.begin(); iter != m_VoiceRxList.end(); ++iter )
    {
        if( pluginType == iter->first && onlineId == iter->second )
        {
            found = true;
            m_VoiceRxList.erase( iter );
            break;
        }
    }

    m_VoicePairRxMutex.unlock();
    if( !found && LogEnabled( eLogVoice ) )
    {
        LogModule( eLogVoice, LOG_ERROR, "PluginBase::%s online id %s not found %s", __func__, onlineId.toHexString().c_str(), DescribePluginType( pluginType ) );
    }

    return found;
}

//============================================================================
bool PluginBase::userNeedsVoicePairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool needsRx{ false };
    m_VoicePairRxMutex.lock();
    for( auto& pair : m_VoiceRxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            needsRx = true;
            break;
        }
    }

    m_VoicePairRxMutex.unlock();
    return needsRx;
}

//============================================================================
int PluginBase::needVoiceRxCount( EPluginType pluginType )
{
    int needCnt{ 0 };
    m_VoicePairRxMutex.lock();
    for( auto& pair : m_VoiceRxList )
    {
        if( pair.first == pluginType )
        {
            needCnt++;
        }
    }

    m_VoicePairRxMutex.unlock();
    return needCnt;
}

//============================================================================
bool PluginBase::getVoiceRxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList )
{
    onlineIdList.clear();
    m_VoicePairRxMutex.lock();
    for( auto& pair : m_VoiceRxList )
    {
        if( pair.first == pluginType )
        {
            onlineIdList.emplace_back( pair.second );
        }
    }

    m_VoicePairRxMutex.unlock();
    return !onlineIdList.empty();
}

//============================================================================
bool PluginBase::isFirstVoicePairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool isFirst{ false };
    m_VoicePairRxMutex.lock();
    for( auto& pair : m_VoiceRxList )
    {
        if( pair.second == onlineId )
        {
            if( pair.first == pluginType )
            {
                isFirst = true;
            }

            break;
        }
    }

    m_VoicePairRxMutex.unlock();
    return isFirst;
}

//============================================================================
void PluginBase::updateRequestMicrophone( EPluginType pluginType, int prevNeedCnt, int needCnt )
{
    if( prevNeedCnt != needCnt )
    {
        if( prevNeedCnt == 0 && needCnt == 1 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputAudioPkts, getMediaModule(), m_MediaSessionId, true);
            LogModule( eLogVoice, LOG_INFO, "PluginBase::%s requested microphone %s", __func__, DescribePluginType( getPluginType() ) );
        }
        else if( prevNeedCnt == 1 && needCnt == 0 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputAudioPkts, getMediaModule(), m_MediaSessionId, false );
            LogModule( eLogVoice, LOG_INFO, "PluginBase::%s NO longer requesting microphone %s", __func__, DescribePluginType( getPluginType() ) );
        }
    }
}

//============================================================================
void PluginBase::updateRequestMixer( EPluginType pluginType, int prevNeedCnt, int needCnt )
{
    if( prevNeedCnt != needCnt )
    {
        if( prevNeedCnt == 0 && needCnt == 1 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputMixer, getMediaModule(), m_MediaSessionId, true );
            LogModule( eLogVoice, LOG_INFO, "PluginBase::%s requested speaker %s", __func__, DescribePluginType( getPluginType() ) );
        }
        else if( prevNeedCnt == 1 && needCnt == 0 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputMixer, getMediaModule(), m_MediaSessionId, false );
            LogModule( eLogVoice, LOG_INFO, "PluginBase::%s NO longer requesting speaker %s", __func__, DescribePluginType( getPluginType() ) );
        }
    }
}

//============================================================================
bool PluginBase::isMyAccessAllowedFromHim( VxGUID& onlineId, EPluginType pluginType )
{
    return m_Engine.isMyAccessAllowedFromHim( onlineId, pluginType );
}

//============================================================================
bool PluginBase::isHisAccessAllowedFromMe( VxGUID& onlineId, EPluginType pluginType )
{
    return m_Engine.isHisAccessAllowedFromMe( onlineId, pluginType );
}

//============================================================================
// Video
//============================================================================

//============================================================================
bool PluginBase::addVideoPairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VideoPairTxMutex.lock();
    for( auto& pair : m_VideoTxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            found = true;
            break;
        }
    }

    if( !found )
    {
        m_VideoTxList.emplace_back( std::make_pair( pluginType, onlineId ) );
    }

    m_VideoPairTxMutex.unlock();
    if( found && LogEnabled( eLogWebCam ) )
    {
        LogModule( eLogWebCam, LOG_ERROR, "PluginBase::%s online id %s already exists", __func__, onlineId.toHexString().c_str() );
    }

    return !found;
}

//============================================================================
bool PluginBase::removeVideoPairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VideoPairTxMutex.lock();
    for( auto iter = m_VideoTxList.begin(); iter != m_VideoTxList.end(); ++iter )
    {
        if( pluginType == iter->first && onlineId == iter->second )
        {
            found = true;
            m_VideoTxList.erase( iter );
            break;
        }
    }

    m_VideoPairTxMutex.unlock();
    if( !found && LogEnabled( eLogWebCam ) )
    {
        LogModule( eLogWebCam, LOG_ERROR, "PluginBase::%s online id %s not found %s", __func__, onlineId.toHexString().c_str(), DescribePluginType( pluginType ) );
    }

    return found;
}

//============================================================================
bool PluginBase::userNeedsVideoPairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool needsTx{ false };
    m_VideoPairTxMutex.lock();
    for( auto& pair : m_VideoTxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            needsTx = true;
            break;
        }
    }

    m_VideoPairTxMutex.unlock();
    return needsTx;
}

//============================================================================
int PluginBase::needVideoTxCount( EPluginType pluginType )
{
    int needCnt{ 0 };
    m_VideoPairTxMutex.lock();
    for( auto& pair : m_VideoTxList )
    {
        if( pair.first == pluginType )
        {
            needCnt++;
        }
    }

    m_VideoPairTxMutex.unlock();
    return needCnt;
}

//============================================================================
bool PluginBase::getVideoTxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList )
{
    onlineIdList.clear();
    m_VideoPairTxMutex.lock();
    for( auto& pair : m_VideoTxList )
    {
        if( pair.first == pluginType )
        {
            onlineIdList.emplace_back( pair.second );
        }
    }

    m_VideoPairTxMutex.unlock();
    return !onlineIdList.empty();
}

//============================================================================
bool PluginBase::isFirstVideoPairTx( EPluginType pluginType, VxGUID& onlineId )
{
    bool isFirst{ false };
    m_VideoPairTxMutex.lock();
    for( auto& pair : m_VideoTxList )
    {
        if( pair.second == onlineId )
        {
            if( pair.first == pluginType )
            {
                isFirst = true;
            }

            break;
        }
    }

    m_VideoPairTxMutex.unlock();
    return isFirst;
}

//============================================================================
bool PluginBase::addVideoPairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VideoPairRxMutex.lock();
    for( auto& pair : m_VideoRxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            found = true;
            break;
        }
    }

    if( !found )
    {
        m_VideoRxList.emplace_back( std::make_pair( pluginType, onlineId ) );
    }

    m_VideoPairRxMutex.unlock();
    if( found && LogEnabled( eLogWebCam ) )
    {
        LogModule( eLogWebCam, LOG_ERROR, "PluginBase::%s online id %s already exists", __func__, onlineId.toHexString().c_str() );
    }

    return !found;

}

//============================================================================
bool PluginBase::removeVideoPairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool found{ false };
    m_VideoPairRxMutex.lock();
    for( auto iter = m_VideoRxList.begin(); iter != m_VideoRxList.end(); ++iter )
    {
        if( pluginType == iter->first && onlineId == iter->second )
        {
            found = true;
            m_VideoRxList.erase( iter );
            break;
        }
    }

    m_VideoPairRxMutex.unlock();
    if( !found && LogEnabled( eLogWebCam ) )
    {
        LogModule( eLogWebCam, LOG_ERROR, "PluginBase::%s online id %s not found %s", __func__, onlineId.toHexString().c_str(), DescribePluginType( pluginType ) );
    }

    return found;
}

//============================================================================
bool PluginBase::userNeedsVideoPairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool needsRx{ false };
    m_VideoPairRxMutex.lock();
    for( auto& pair : m_VideoRxList )
    {
        if( pair.first == pluginType && pair.second == onlineId )
        {
            needsRx = true;
            break;
        }
    }

    m_VideoPairRxMutex.unlock();
    return needsRx;
}

//============================================================================
int PluginBase::needVideoRxCount( EPluginType pluginType )
{
    int needCnt{ 0 };
    m_VideoPairRxMutex.lock();
    for( auto& pair : m_VideoRxList )
    {
        if( pair.first == pluginType )
        {
            needCnt++;
        }
    }

    m_VideoPairRxMutex.unlock();
    return needCnt;
}

//============================================================================
bool PluginBase::getVideoRxList( EPluginType pluginType, std::vector<VxGUID>& onlineIdList )
{
    onlineIdList.clear();
    m_VideoPairRxMutex.lock();
    for( auto& pair : m_VideoRxList )
    {
        if( pair.first == pluginType )
        {
            onlineIdList.emplace_back( pair.second );
        }
    }

    m_VideoPairRxMutex.unlock();
    return !onlineIdList.empty();
}

//============================================================================
bool PluginBase::isFirstVideoPairRx( EPluginType pluginType, VxGUID& onlineId )
{
    bool isFirst{ false };
    m_VideoPairRxMutex.lock();
    for( auto& pair : m_VideoRxList )
    {
        if( pair.second == onlineId )
        {
            if( pair.first == pluginType )
            {
                isFirst = true;
            }

            break;
        }
    }

    m_VideoPairRxMutex.unlock();
    return isFirst;
}

//============================================================================
void PluginBase::updateRequestVideoCapture( EPluginType pluginType, int prevNeedCnt, int needCnt )
{
    if( prevNeedCnt != needCnt )
    {
        if( prevNeedCnt == 0 && needCnt == 1 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputVideoPkts, getMediaModule(), m_MediaSessionId, true );
            LogModule( eLogWebCam, LOG_INFO, "PluginBase::%s requested cam send pkts %s", __func__, DescribePluginType( getPluginType() ) );
        }
        else if( prevNeedCnt == 1 && needCnt == 0 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputVideoPkts, getMediaModule(), m_MediaSessionId, false );
            LogModule( eLogWebCam, LOG_INFO, "PluginBase::%s NO longer requesting cam send pkts %s", __func__, DescribePluginType( getPluginType() ) );
        }
    }
}

//============================================================================
void PluginBase::updateRequestVideoPlay( EPluginType pluginType, int prevNeedCnt, int needCnt )
{
    if( prevNeedCnt != needCnt )
    {
        if( prevNeedCnt == 0 && needCnt == 1 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputVideoJpg, getMediaModule(), m_MediaSessionId, true );
            LogModule( eLogWebCam, LOG_INFO, "PluginBase::%s requested cam play %s", __func__, DescribePluginType( getPluginType() ) );
        }
        else if( prevNeedCnt == 1 && needCnt == 0 )
        {
            m_PluginMgr.pluginApiWantMediaInput( getPluginType(), eMediaInputVideoJpg, getMediaModule(), m_MediaSessionId, false );
            LogModule( eLogWebCam, LOG_INFO, "PluginBase::%s NO longer requesting cam play %s", __func__, DescribePluginType( getPluginType() ) );
        }
    }
}

//============================================================================
void PluginBase::sendSessionStop( std::shared_ptr<VxSktBase>& sktBase, PluginSessionBase* sessionBase )
{
    if( sessionBase && sktBase.get() && sktBase->isConnected() )
    {
        PktSessionStopReq pktStop;
        VxGUID srcOnlineId = sessionBase->getSendToId();

        pktStop.setLclSessionId( sessionBase->getLclSessionId() );
        pktStop.setRmtSessionId( sessionBase->getRmtSessionId() );
        if( LogEnabled( eLogSession ) ) LogModule( eLogSession, LOG_VERBOSE, "PluginBase::%s lcl session id %s rmt session id %s to %s", __func__,
            sessionBase->getLclSessionId().toHexString().c_str(), sessionBase->getRmtSessionId().toHexString().c_str(), m_Engine.describeUser( sessionBase->getSendToId() ).c_str() );
        txPacket( srcOnlineId, sktBase, &pktStop, getPluginType() );
    }
}