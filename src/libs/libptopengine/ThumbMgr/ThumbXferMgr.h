#pragma once
//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <AssetBase/AssetBaseXferMgr.h>

class PktThumbSendReq;
class PktThumbSendReply;
class PktThumbChunkReq;
class PktThumbSendCompleteReq;
class PktThumbListReply;

class ThumbXferMgr : public AssetBaseXferMgr
{
public:
	ThumbXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface );
	virtual ~ThumbXferMgr() = default;

    virtual void				sendToGuiAssetAction( EAssetAction assetAction, VxGUID& assetId, int pos0to100000 ) override {}; // dont send so is not in uploads/downloads

    virtual void				onPktThumbGetReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktThumbSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktThumbChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktThumbChunkReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbGetCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktThumbSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktThumbSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktThumbXferErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

    virtual PktBaseGetReq*			    createPktBaseGetReq( void ) override;
    virtual PktBaseGetReply*			createPktBaseGetReply( void ) override;
    virtual PktBaseSendReq*			    createPktBaseSendReq( void ) override;
    virtual PktBaseSendReply*			createPktBaseSendReply( void ) override;
    virtual PktBaseChunkReq*			createPktBaseChunkReq( void ) override;
    virtual PktBaseChunkReply*			createPktBaseChunkReply( void ) override;
    virtual PktBaseGetCompleteReq*		createPktBaseGetCompleteReq( void ) override;
    virtual PktBaseGetCompleteReply*	createPktBaseGetCompleteReply( void ) override;
    virtual PktBaseSendCompleteReq*		createPktBaseSendCompleteReq( void ) override;
    virtual PktBaseSendCompleteReply*	createPktBaseSendCompleteReply( void ) override;
    virtual PktBaseXferErr*			    createPktBaseXferErr( void ) override;

    virtual bool				        requestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, VxGUID& thumbId, bool tmpThumb = false );
};



