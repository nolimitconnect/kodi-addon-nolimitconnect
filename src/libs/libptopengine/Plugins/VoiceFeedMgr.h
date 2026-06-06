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

class P2PEngine;
class PktVoiceReq;
class PluginBase;
class PluginMgr;
class PluginSessionMgr;
class VxNetIdent;
class VxSktBase;
class VxPktHdr;

class VoiceFeedMgr
{
public:
	VoiceFeedMgr( P2PEngine& engine, PluginBase& plugin, PluginSessionMgr& sessionMgr );

	virtual void				onPktVoiceReq				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktVoiceReply				( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	virtual void				callbackOpusPkt( PktVoiceReq * pktOpusAudio );
	virtual void				callbackAudioOutSpaceAvail( int freeSpaceLenBytes );

	void						stopAllSessions( void );

	void						enableAudioCapture( bool enable, VxGUID onlineId );

	void						enableAudioReceive( bool enable, VxGUID onlineId );

protected:
    P2PEngine&                  m_Engine;
	PluginBase&					m_Plugin;
	PluginMgr&					m_PluginMgr;
	PluginSessionMgr&			m_SessionMgr;
};

