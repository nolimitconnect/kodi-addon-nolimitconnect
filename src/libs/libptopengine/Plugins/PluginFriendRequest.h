#pragma once
//============================================================================
// Copyright (C) 2025 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseFilesServer.h"

class PluginFriendRequest : public PluginBase
{
public:
	PluginFriendRequest( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* netIdent, EPluginType pluginType );
	virtual ~PluginFriendRequest() = default;

	EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

	void						onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase ) override {};
	void						onConnectionLost(std::shared_ptr<VxSktBase> &) override {};
	void						onContactOnlineStatusChange(ConnectId&,bool) override {};
	void						replaceConnection(VxNetIdent *,std::shared_ptr<VxSktBase> &,std::shared_ptr<VxSktBase> &) override {};	

	// NOTE: netIdent might be null for friend request
	void						onPktFriendRequestReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;
	void						onPktFriendRequestReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

	bool						fromGuiSendFriendRequest( VxGUID& onlineId, std::string& requestText, EFriendState myFriendshipToHim );

protected:

};


