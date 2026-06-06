#pragma once
//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseMultimedia.h"

#include "VoiceFeedMgr.h"
#include "VideoFeedMgr.h"

#include <AssetMgr/AssetInfo.h>
#include <Connections/IConnectRequest.h>

class PluginMessenger : public PluginBaseMultimedia, public IConnectRequestCallback
{
public:
	PluginMessenger( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
	virtual ~PluginMessenger() = default;

    EMediaModule				getMediaModule( void ) override		{ return eMediaModuleMessenger; }

	void						wantAssetXferCallbacks( AssetXferCallback* client, bool enable );

	void						fromGuiJoinHost( HostedId& adminId, VxGUID& sessionId, std::string& hostUrl ) override;

protected:
	bool						onUrlActionQueryIdSuccess( VxGUID& sessionId, std::string& url, VxGUID& onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
	void						onUrlActionQueryIdFail( VxGUID& sessionId, std::string& url, enum ERunTestStatus testStatus, 
                                                       enum  EConnectReason connectReason = eConnectReasonUnknown, ECommErr commErr = eCommErrNone ) override {};

	bool						onContactHandshaking( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override { return true; };
	void						onHandshakeTimeout( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};
	void						onContactSessionDone( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

	void						onConnectRequestFail( VxGUID& sessionId, VxGUID& onlineId, EConnectStatus connectStatus, 
                                                      enum EConnectReason connectReason = eConnectReasonUnknown, enum ECommErr commErr = eCommErrNone ) override {};
	void						onContactDisconnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override {};

    /// returns false if one time use and packet has been sent. Connect Manager will disconnect if nobody else needs the connection
	bool						onContactConnected( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId, enum EConnectReason connectReason = eConnectReasonUnknown ) override;

	bool						sendWasInvited( VxGUID& sessionId, std::shared_ptr<VxSktBase>& sktBase, VxGUID onlineId );

	AssetBaseInfo				m_AssetInfo;
};



