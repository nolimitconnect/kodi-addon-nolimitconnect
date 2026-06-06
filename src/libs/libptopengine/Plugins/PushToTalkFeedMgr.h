#pragma once
//============================================================================
// Copyright (C) 2014 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <GuiInterface/IDefs.h>

#include <CoreLib/MediaCallbackInterface.h>
#include <CoreLib/VxGUID.h>
#include <CoreLib/VxGUIDList.h>

#include <memory>

class ConnectId;
class P2PEngine;
class PluginBase;
class PluginMgr;
class PluginSessionMgr;
class VxNetIdent;
class VxSktBase;
class VxPktHdr;
class PktPushToTalkReq;

class PushToTalkFeedMgr
{
public:
	PushToTalkFeedMgr( P2PEngine& engine, PluginBase& plugin, PluginSessionMgr& sessionMgr );

	virtual bool				fromGuiPushToTalk( VxGUID& onlineId, bool enableTalk, std::shared_ptr<VxSktBase>& sktBase );

	virtual void				onPktPushToTalkReq					( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktPushToTalkReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktPushToTalkStart				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktPushToTalkStop                 ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual void				onPktVoiceReq						( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktVoiceReply						( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

    virtual void				callbackOpusPkt( PktVoiceReq* pktOpusAudio );
	virtual void				callbackAudioOutSpaceAvail( int freeSpaceLenBytes );

	virtual bool				addPushToTalkUserTx( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase );
	virtual bool				addPushToTalkUserRx( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase );
	virtual bool				removePushToTalkUserTx( VxGUID& onlineId );
	virtual bool				removePushToTalkUserRx( VxGUID& onlineId);
	virtual void				onContactWentOffline( VxNetIdent* netIdent, std::shared_ptr<VxSktBase>& sktBase );

	void					onContactOnlineStatusChange( ConnectId& connectId, bool isOnline );
	void					onSessionEnded( VxGUID& onlineId );

	virtual bool				sendPushToTalkStart( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase );
	virtual bool				sendPushToTalkStop( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase );
	virtual bool				sendPushToTalkReq( VxGUID& onlineId, std::shared_ptr<VxSktBase>& sktBase );

protected:
	bool						enableAudioCapture( bool enable, VxGUID& onlineId, EMediaModule mediaModule, std::shared_ptr<VxSktBase>& sktBase );
	void					cleanupPushToTalkUser( VxGUID& onlineId );
	void						updatePushToTalkStatus( VxGUID& onlineId );
	void						startRxSession( std::shared_ptr<VxSktBase>& sktBase, VxGUID& onlineId );

    P2PEngine&                  m_Engine;
	PluginBase&					m_Plugin;
	PluginMgr&					m_PluginMgr;
	PluginSessionMgr&			m_SessionMgr;
};

