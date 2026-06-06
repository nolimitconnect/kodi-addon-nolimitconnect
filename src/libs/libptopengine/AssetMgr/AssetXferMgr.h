#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <AssetBase/AssetBaseXferMgr.h>

class PktAssetSendReq;
class PktAssetSendReply;
class PktAssetChunkReq;
class PktAssetSendCompleteReq;
class PktAssetListReply;

class AssetBaseMgr;

class AssetXferMgr : public AssetBaseXferMgr
{
public:
	AssetXferMgr( P2PEngine& engine, AssetBaseMgr& assetMgr, BaseXferInterface& xferInterface );
	virtual ~AssetXferMgr() = default;

    virtual void				onPktAssetGetReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktAssetGetReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetSendReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetSendReply			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetChunkReq			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetChunkReply		( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktAssetGetCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktAssetGetCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetSendCompleteReq	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetSendCompleteReply	( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
	virtual void				onPktAssetXferErr			( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

    virtual PktBaseGetReq*			    createPktBaseGetReq( void ) override;
    virtual PktBaseGetReply*			createPktBaseGetReply( void ) override;
    virtual PktBaseSendReq*			    createPktBaseSendReq( void ) override;
    virtual PktBaseSendReply*			createPktBaseSendReply( void ) override;
    virtual PktBaseChunkReq*			createPktBaseChunkReq( void ) override;
    virtual PktBaseChunkReply*          createPktBaseChunkReply( void ) override;
    virtual PktBaseGetCompleteReq*		createPktBaseGetCompleteReq( void ) override;
    virtual PktBaseGetCompleteReply*	createPktBaseGetCompleteReply( void ) override;
    virtual PktBaseSendCompleteReq*		createPktBaseSendCompleteReq( void ) override;
    virtual PktBaseSendCompleteReply*	createPktBaseSendCompleteReply( void ) override;
    virtual PktBaseXferErr*			    createPktBaseXferErr( void ) override;


};



