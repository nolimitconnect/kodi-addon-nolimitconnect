#ifndef VIDEO_FEED_MGR_H
#define VIDEO_FEED_MGR_H
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
class PluginBase;
class PluginMgr;
class PluginSessionMgr;
class VxNetIdent;
class VxSktBase;
class VxPktHdr;

class VideoFeedMgr
{
public:
	VideoFeedMgr( P2PEngine& engine, PluginBase& plugin, PluginSessionMgr& sessionMgr );
	virtual ~VideoFeedMgr() = default;

    virtual bool                fromGuiStartPluginSession( bool pluginIsLocked, EMediaModule mediaModule, VxGUID onlineId, bool wantCamCapture = true );
    virtual void				fromGuiStopPluginSession( bool pluginIsLocked, EMediaModule mediaModule, VxGUID onlineId, bool wantCamCapture = true );

	virtual void				onPktVideoFeedReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktVideoFeedStatus		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktVideoFeedPic			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktVideoFeedPicChunk		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktVideoFeedPicAck		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

	//virtual void				callbackVideoJpg(	void * userData, VxGUID& feedId, uint8_t * jpgData, uint32_t jpgDataLen, int motion0to100000 );
	virtual void				callbackVideoPktPic( VxGUID& feedId, PktVideoFeedPic * pktVid, int pktsInSequence, int thisPktNum );
	virtual void				callbackVideoPktPicChunk( VxGUID& feedId, PktVideoFeedPicChunk * pktVid, int pktsInSequence, int thisPktNum );

	void						stopAllSessions( EMediaModule mediaModule, EPluginType pluginType );

protected:
	void						enableVideoCapture( bool bStart, VxGUID& onlineId, EMediaModule mediaModule, bool wantCamCapture );

    P2PEngine&                  m_Engine;
	PluginBase&					m_Plugin;
	PluginMgr&					m_PluginMgr;
	PluginSessionMgr&			m_SessionMgr;
	VxGUIDList					m_GuidList;
	bool						m_CamServerEnabled;
	bool						m_VideoPktsRequested;
	bool						m_VideoJpgRequesed;
};

#endif // VIDEO_FEED_MGR_H
