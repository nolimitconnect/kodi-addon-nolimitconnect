//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginMessenger.h"

#include "PluginMgr.h"

#include <GuiInterface/IToGui.h>
#include <P2PEngine/P2PEngine.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxPtopUrl.h>

#include <PktLib/PktsVideoFeed.h>
#include <PktLib/PktsMultiSession.h>
#include <PktLib/PktsTodGame.h>

#include <memory.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif //_MSC_VER

//============================================================================
PluginMessenger::PluginMessenger( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType )
: PluginBaseMultimedia( engine, pluginMgr, myIdent, pluginType )
{
	setPluginType( ePluginTypeMessenger );
}

//============================================================================
void PluginMessenger::wantAssetXferCallbacks( AssetXferCallback* client, bool enable )
{
	m_AssetXferMgr.wantAssetXferCallbacks( client, enable );
}

//============================================================================
void PluginMessenger::fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl )
{
	// peer to peer
	VxPtopUrl ptopUrl( hostUrl );
	if( !ptopUrl.isValid() )
	{
		return;
	}

	VxGUID onlineId = ptopUrl.getOnlineId();
	EFriendState myFriendshipToHim;
	EFriendState hisFriendshipToMe;
	m_Engine.getBigListMgr().getFriendships( onlineId, myFriendshipToHim, hisFriendshipToMe );

	if( myFriendshipToHim == eFriendStateIgnore )
	{
		LogMsg( LOG_ERROR, "PluginMessenger::%s invite from ignored user url %s", __func__, hostUrl.c_str() );
		return;
	}

    if( m_Engine.getConnectIdListMgr().isUserOnline( onlineId ) )
	{
		LogMsg( LOG_VERBOSE, "PluginMessenger::%s user is already online", __func__ );
		std::shared_ptr<VxSktBase> sendSktBase = m_Engine.getConnectIdListMgr().findAnyUserOnlineConnection( onlineId );
		if( sendSktBase.get() != nullptr )
		{
			sendWasInvited( sessionId, sendSktBase, onlineId );
			return;
		}	
	}

	std::shared_ptr<VxSktBase> sktBase( nullptr );
	EConnectStatus connectStatus = m_Engine.getConnectionMgr().requestConnection( sessionId, ptopUrl.getUrl(), onlineId, this, sktBase, eConnectReasonPeerInvite );
	if( eConnectStatusConnectFailed == connectStatus )
	{
		LogMsg( LOG_VERBOSE, "PluginMessenger::%s failed to connect url %s", __func__, hostUrl.c_str() );
	}
}

//============================================================================
bool PluginMessenger::onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason )
{
	if( connectReason == eConnectReasonPeerInvite )
	{
		sendWasInvited( sessionId, sktBase, onlineId );
	}

	return true;
}

//============================================================================
bool PluginMessenger::sendWasInvited( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId )
{
	LogMsg( LOG_VERBOSE, "PluginMessenger::%s", __func__ );

	m_AssetInfo.setAssetType( eAssetTypeChatText );

	std::string message( "Invite Accepted\n" );
	m_AssetInfo.setAssetName( message.c_str() );
	m_AssetInfo.setAssetLength( message.length() );

	m_AssetInfo.generateNewUniqueId();
	m_AssetInfo.setPluginType( getPluginType() );
	
	m_AssetInfo.setCreationTime( GetTimeStampMs() );
	m_AssetInfo.setCreatorId( m_Engine.getMyOnlineId() );
	m_AssetInfo.setHistoryId( m_Engine.getMyOnlineId());

	m_AssetInfo.setOnlineId( onlineId );

	return m_Engine.fromGuiAssetAction( eAssetActionAddAssetAndSend, m_AssetInfo );
}
