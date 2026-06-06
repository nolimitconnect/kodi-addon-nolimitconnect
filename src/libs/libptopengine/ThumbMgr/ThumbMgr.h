#pragma once
//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ThumbInfoDb.h"

#include <AssetBase/AssetBaseMgr.h>
#include <GuiInterface/IDefs.h>

#include <CoreLib/VxMutex.h>

#include <memory>

class PktFileListReply;

class ThumbInfo;
class ThumbHistoryMgr;
class ThumbCallbackInterface;
class VxNetIdent;
class VxPktHdr;
class VxSktBase;
class BigListInfo;

class ThumbMgr : public AssetBaseMgr
{
    const int THUMB_DB_VERSION = 1;
public:
	ThumbMgr( P2PEngine& engine, const char* dbName, const char* dbStateName );
	virtual ~ThumbMgr() = default;

    void                        addThumbMgrClient( ThumbCallbackInterface * client, bool enable );

    void                        onPluginsInitialized( void ) override;
    bool				        fromGuiThumbCreated( ThumbInfo& thumbInfo );
    bool				        fromGuiThumbUpdated( ThumbInfo& thumbInfo );
    virtual bool			    fromGuiRequestPluginThumb( VxNetIdent* netIdent, EPluginType pluginType, VxGUID& thumbId );
    std::string				    fromGuiGetThumbFile( VxGUID& thumbId );
    virtual uint64_t			fromGuiClearCache( ECacheType cacheType );

    virtual void				announceAssetAdded( AssetBaseInfo* assetInfo, bool resourceLocked = false ) override;
    virtual void				announceAssetUpdated( AssetBaseInfo* assetInfo ) override;
    virtual void				announceAssetRemoved( AssetBaseInfo* assetInfo, bool resourceLocked = false ) override;
    virtual void				announceAssetXferState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState assetSendState, int param ) override;

    virtual void				queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EHostType hostType );
    virtual void				queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EPluginType pluginType );
    virtual void				queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EHostType hostType, VxGUID& thumbId, int64_t thumbModifiedTime );
    virtual void				queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EPluginType pluginType, VxGUID& thumbId, int64_t thumbModifiedTime );

    virtual void				queryMediaThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxGUID& srcOnlineId, EPluginType pluginType, VxGUID& thumbId );

    // packet handlers
    virtual void				onPktThumbGetReq            ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbGetReply          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbSendReq           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbSendReply         ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbChunkReq          ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbChunkReply        ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbGetCompleteReq    ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbGetCompleteReply  ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbSendCompleteReq   ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbSendCompleteReply ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );
    virtual void				onPktThumbXferErr           ( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent );

    virtual bool                ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EPluginType pluginType, VxGUID& thumbId, bool tmpThumb = false );
    virtual bool                requestThumbs( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent );

    virtual void                deleteThumb( VxGUID& thumbId );

protected:
    virtual bool				addAsset( AssetBaseInfo& assetInfo, AssetBaseInfo*& retCreatedAsset ) override;

    AssetBaseInfo*              createAssetInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen ) override;
    AssetBaseInfo*              createAssetInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId ) override;
    AssetBaseInfo*              createAssetInfo( AssetBaseInfo& assetInfo ) override;
    AssetBaseInfo*				createAssetInfo( FileInfo& fileInfo ) override;

    ThumbInfo*                  lookupThumbInfo( VxGUID& thumbId, int64_t thumbModifiedTime = 0 );
    void				        announceThumbAdded( ThumbInfo& thumbInfo );
    void				        announceThumbUpdated( ThumbInfo& thumbInfo );
    bool                        saveToDatabase( ThumbInfo& thumbInfo );
    bool                        isThumbUpToDate( VxGUID& thumbId, int64_t thumbModifiedTime );

    ThumbInfoDb&                m_ThumbInfoDb;
    std::vector<AssetBaseInfo*>&	m_ThumbInfoList;
    VxMutex						m_ThumbInfoMutex;
    bool                        m_ThumbListInitialized{ false };

    std::vector<ThumbCallbackInterface *> m_ThumbClients;
    VxMutex						m_ThumbClientMutex;
};

