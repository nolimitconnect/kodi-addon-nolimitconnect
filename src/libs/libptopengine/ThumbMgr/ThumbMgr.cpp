//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ThumbMgr.h"
#include "ThumbInfo.h"
#include "ThumbInfoDb.h"
#include "ThumbCallbackInterface.h"

#include <P2PEngine/P2PEngine.h>
#include <Plugins/PluginMgr.h>
#include <Plugins/PluginBase.h>
#include <BigListLib/BigListInfo.h>
#include <GuiInterface/IToGui.h>

#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsFileList.h>

#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>

#include <algorithm>
#include <time.h>

//============================================================================
ThumbMgr::ThumbMgr( P2PEngine& engine, const char* dbName, const char* dbStateName )
: AssetBaseMgr( engine, dbName, dbStateName,  eAssetMgrTypeThumb )
, m_ThumbInfoDb( dynamic_cast<ThumbInfoDb&>(AssetBaseMgr::getAssetInfoDb()) )
, m_ThumbInfoList( getAssetBaseInfoList() )
{
}

//============================================================================
AssetBaseInfo* ThumbMgr::createAssetInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t assetLen )
{
    if( eAssetTypeThumbnail == assetType )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::createAssetInfo creating thumbnail asset of asset type %s", DescribeAssetType( assetType ) );
    }

    ThumbInfo* assetInfo = new ThumbInfo( fileName, fileNameAndPath, assetLen );
    assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* ThumbMgr::createAssetInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t assetLen, VxGUID& assetId )
{
    if( eAssetTypeThumbnail == assetType )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::createAssetInfo creating thumbnail asset of asset type %s", DescribeAssetType( assetType ) );
    }

    ThumbInfo* assetInfo = new ThumbInfo( fileName, fileNameAndPath, assetLen, assetId );
    assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* ThumbMgr::createAssetInfo( AssetBaseInfo& assetInfo )
{
    ThumbInfo* assetInfoNew = new ThumbInfo( assetInfo );
    assetInfoNew->assureHasCreatorId();
    return assetInfoNew;
}

//============================================================================
AssetBaseInfo* ThumbMgr::createAssetInfo( FileInfo& fileInfo )
{
    ThumbInfo* assetInfoNew = new ThumbInfo( fileInfo );
    assetInfoNew->assureHasCreatorId();
    return assetInfoNew;
}

//============================================================================
void ThumbMgr::onPluginsInitialized( void )
{
    // dont call AssetBaseMgr::fromGuiUserLoggedOn because we never generate sha hash for thumbnails
    if( !m_Initialized )
    {
        m_Initialized = true;
        // user specific directory should be set
        std::string dbFileName = VxGetSettingsDirectory();
        dbFileName += m_ThumbInfoDb.getDatabaseName(); 
        lockResources();
        m_ThumbInfoDb.dbShutdown();
        m_ThumbInfoDb.dbStartup( THUMB_DB_VERSION, dbFileName );

        clearAssetInfoList();
        m_ThumbInfoDb.getAllAssets( m_ThumbInfoList );
        for( auto asset : m_ThumbInfoList )
        {
            announceAssetAdded( asset, true );
        }

        m_ThumbListInitialized = true;
        unlockResources();
    }
}

//============================================================================
void ThumbMgr::addThumbMgrClient( ThumbCallbackInterface * client, bool enable )
{
    lockClientList();
    if( enable )
    {
        m_ThumbClients.emplace_back( client );
    }
    else
    {
        std::vector<ThumbCallbackInterface *>::iterator iter;
        for( iter = m_ThumbClients.begin(); iter != m_ThumbClients.end(); ++iter )
        {
            if( *iter == client )
            {
                m_ThumbClients.erase( iter );
                break;
            }
        }
    }

    unlockClientList();
}

//============================================================================
bool ThumbMgr::addAsset( AssetBaseInfo& assetInfo, AssetBaseInfo*& retCreatedAsset )
{
    bool result = AssetBaseMgr::addAsset( assetInfo, retCreatedAsset );
    if( result )
    {
        announceAssetAdded( retCreatedAsset );
    }

    return result;
}

//============================================================================
void ThumbMgr::announceAssetAdded( AssetBaseInfo* assetInfo, bool resourceLocked )
{
    if( !assetInfo || !assetInfo->isValidThumbnail() )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::announceThumbAdded invalid thumbnil" );
        vx_assert( false );
    }

    AssetBaseMgr::announceAssetAdded( assetInfo, resourceLocked );
    ThumbInfo* thumbInfo = dynamic_cast<ThumbInfo*>( assetInfo );
    if( thumbInfo )
    {
	    if(LogEnabled(eLogThumbnail))LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbMgr::%s start", __func__ );
	
	    lockClientList();
	    for( auto& client : m_ThumbClients )
	    {
		    client->callbackThumbAdded( thumbInfo );
	    }

	    unlockClientList();
        if( LogEnabled( eLogThumbnail ) )LogModule( eLogThumbnail, LOG_VERBOSE, "ThumbMgr::%s done", __func__ );
    }
    else
    {
        LogMsg( LOG_ERROR, "ThumbMgr::announceAssetAdded dynamic_cast failed" );
    }
}

//============================================================================
void ThumbMgr::announceAssetUpdated( AssetBaseInfo* assetInfo )
{
    if( !assetInfo || !assetInfo->isValidThumbnail() )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::announceAssetUpdated invalid thumbnil" );
        vx_assert( false );
    }

    AssetBaseMgr::announceAssetUpdated( assetInfo );
    ThumbInfo* thumbInfo = dynamic_cast<ThumbInfo*>( assetInfo );
    if( thumbInfo )
    {
        lockClientList();
 	    for( auto& client : m_ThumbClients )
	    {
            client->callbackThumbUpdated( thumbInfo );
        }

        unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "ThumbMgr::announceAssetRemoved dynamic_cast failed" );
    }
}

//============================================================================
void ThumbMgr::announceAssetRemoved( AssetBaseInfo* assetInfo, bool resourceLocked )
{
    AssetBaseMgr::announceAssetRemoved( assetInfo, resourceLocked );
    ThumbInfo* thumbInfo = dynamic_cast<ThumbInfo*>( assetInfo );
    if( thumbInfo && thumbInfo->isThumbAsset() )
    {
	    lockClientList();
	    for( auto& client : m_ThumbClients )
	    {
		    client->callbackThumbRemoved( thumbInfo->getThumbId() );
	    }

	    unlockClientList();
    }
    else
    {
        LogMsg( LOG_ERROR, "ThumbMgr::announceAssetRemoved dynamic_cast failed" );
    }
}

//============================================================================
void ThumbMgr::announceAssetXferState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState assetSendState, int param )
{
    AssetBaseMgr::announceAssetXferState( sendToId, assetUniqueId, assetSendState, param );

	LogMsg( LOG_INFO, "ThumbMgr::announceAssetXferState state %d start", assetSendState );
	lockClientList();

	for( auto& client : m_ThumbClients )
	{
		client->callbackAssetSendState( sendToId, assetUniqueId, assetSendState, param );
	}

	unlockClientList();
	LogMsg( LOG_INFO, "ThumbMgr::announceAssetXferState state %d done", assetSendState );
}

//============================================================================
void ThumbMgr::announceThumbAdded( ThumbInfo& thumbInfo )
{
    lockClientList();
	for( auto& client : m_ThumbClients )
	{
        client->callbackThumbAdded( &thumbInfo );
    }

    unlockClientList();
}

//============================================================================
void ThumbMgr::announceThumbUpdated( ThumbInfo& thumbInfo )
{
    if( !thumbInfo.isValidThumbnail() )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::announceThumbUpdated invalid thumbnil" );
        vx_assert( false );
    }

    lockClientList();
	for( auto& client : m_ThumbClients )
	{
        client->callbackThumbUpdated( &thumbInfo );
    }

    unlockClientList();
}

//============================================================================
ThumbInfo* ThumbMgr::lookupThumbInfo( VxGUID& thumbId, int64_t thumbModifiedTime )
{
    m_ThumbInfoMutex.lock();
    for( AssetBaseInfo* thumbInfo : m_ThumbInfoList )
    {
        if( thumbInfo->getThumbId() == thumbId && ( isEmoticonThumbnail( thumbId ) || 0 == thumbModifiedTime || thumbModifiedTime <= thumbInfo->getInfoModifiedTime() ) )
        {
            m_ThumbInfoMutex.unlock();
            return dynamic_cast<ThumbInfo*>(thumbInfo);
        }
    }

    m_ThumbInfoMutex.unlock();
    return nullptr;
}

//============================================================================
bool ThumbMgr::fromGuiThumbCreated( ThumbInfo& thumbInfo )
{
    AssetBaseInfo* existingAsset = findAsset( thumbInfo.getAssetUniqueId() );
    if( existingAsset )
    {
        *existingAsset = thumbInfo;
        updateDatabase( existingAsset );
        announceAssetUpdated( existingAsset );
        return true;
    }

    // thumbInfo will be destroyed.. only use the object created by addAsset
    AssetBaseInfo* createdThumbInfo = nullptr;
    if( AssetBaseMgr::addAsset( thumbInfo, createdThumbInfo ) && createdThumbInfo )
    {
        ThumbInfo* newThumbInfo = dynamic_cast<ThumbInfo*>(createdThumbInfo);
        if( newThumbInfo )
        {
            if( saveToDatabase( *newThumbInfo ) )
            {
                announceThumbAdded( *newThumbInfo );
                return true;
            }
            else
            {
                LogMsg( LOG_ERROR, "ThumbMgr::fromGuiThumbCreated failed save to db" );
            }
        }
        else
        {
            LogMsg( LOG_ERROR, "ThumbMgr::fromGuiThumbCreated failed cast to ThumbInfo" );
        }
    }
    else
    {
        LogMsg( LOG_ERROR, "ThumbMgr::fromGuiThumbCreated failed add asset" );
    }

    return false;
}

//============================================================================
bool ThumbMgr::fromGuiThumbUpdated( ThumbInfo& thumbInfo )
{
    if( AssetBaseMgr::updateAsset( thumbInfo ) )
    {
        if( saveToDatabase( thumbInfo ) )
        {
            announceThumbUpdated( thumbInfo );
            return true;
        }
    }

    return false;
}

//===========================================================================
std::string ThumbMgr::fromGuiGetThumbFile( VxGUID& thumbId )
{
    std::string fileName;
    m_ThumbInfoMutex.lock();
    for( AssetBaseInfo* thumbInfo : m_ThumbInfoList )
    {
        if( thumbInfo->getThumbId() == thumbId )
        {
            fileName = thumbInfo->getAssetNameAndPath();
            break;
        }
    }

    m_ThumbInfoMutex.unlock();
    return fileName;
}

//============================================================================
void ThumbMgr::queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EHostType hostType )
{
    if( eHostTypeUnknown != hostType && netIdent->hasThumbId( hostType ) )
    {
        queryThumbIfNeeded( sktBase, netIdent, hostType, netIdent->getThumbId( hostType ), netIdent->getHostOrThumbModifiedTime( hostType ) );
    }   
}

//============================================================================
void ThumbMgr::queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EPluginType pluginType )
{
    EHostType hostType = PluginTypeToHostType( pluginType );
    if( eHostTypeUnknown != hostType && netIdent->hasThumbId( hostType ) )
    {
        queryThumbIfNeeded( sktBase, netIdent, hostType, netIdent->getThumbId( hostType ), netIdent->getHostOrThumbModifiedTime( hostType ) );
    }    
}

//============================================================================
void ThumbMgr::queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EHostType hostType, VxGUID &thumbId, int64_t thumbModifiedTime )
{
    if( !lookupThumbInfo( thumbId, thumbModifiedTime ) )
    {
        EPluginType pluginType = HostTypeToClientPlugin( hostType );
        if( pluginType != ePluginTypeInvalid )
        {
            queryThumbIfNeeded( sktBase, netIdent, pluginType, thumbId, thumbModifiedTime );
        }
    }
}

//============================================================================
void ThumbMgr::queryThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EPluginType pluginType, VxGUID &thumbId, int64_t thumbModifiedTime )
{
    if( !lookupThumbInfo( thumbId, thumbModifiedTime ) )
    {
        ptopEngineRequestPluginThumb( sktBase, netIdent, pluginType, thumbId );
    }
}

//============================================================================
bool ThumbMgr::saveToDatabase( ThumbInfo& thumbInfo )
{
    lockResources();

    bool result = m_ThumbInfoDb.saveToDatabase( thumbInfo );

    unlockResources();
    return result;
}

//============================================================================
void ThumbMgr::onPktThumbGetReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbGetReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbSendReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbSendReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbChunkReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbChunkReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbGetCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbGetCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbSendCompleteReq( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbSendCompleteReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
void ThumbMgr::onPktThumbXferErr( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent )
{
}

//============================================================================
bool ThumbMgr::fromGuiRequestPluginThumb( VxNetIdent* netIdent, EPluginType pluginType, VxGUID& thumbId )
{
    if( !netIdent || ePluginTypeInvalid == pluginType || !thumbId.isValid() )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::requestPluginThumb invalid param " );
        vx_assert( false );
        return false;
    }

    if( isEmoticonThumbnail( thumbId ) )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::requestPluginThumb emoticon thumbnails must be generated by gui " );
        return false;
    }

    if( IsHostPluginType( pluginType ) )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::requestPluginThumb You must request thumb using Client plugin instead of Host plugin %s ", DescribePluginType( pluginType ) );
        vx_assert( false );
        return false;
    }

    PluginBase* plugin = m_Engine.getPluginMgr().getPlugin( pluginType );
    if( plugin )
    {
        return plugin->fromGuiRequestPluginThumb( netIdent, thumbId );
    }
    else
    {
        LogMsg( LOG_ERROR, "ThumbMgr::requestPluginThumb invalid plugin " );
        vx_assert( false );
        return false;
    }
}

//============================================================================
bool ThumbMgr::ptopEngineRequestPluginThumb( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent, EPluginType pluginType, VxGUID& thumbId, bool tmpThumb )
{
    if( !netIdent || ePluginTypeInvalid == pluginType || !thumbId.isValid() )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::%s invalid param ", __func__ );
        vx_assert( false );
        return false;
    }

    if( IsHostPluginType( pluginType ) )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::%s You must request thumb using Client plugin instead of Host plugin %s ", 
                __func__, DescribePluginType( pluginType ) );
        vx_assert( false );
        return false;
    }

    PluginBase* plugin = m_Engine.getPluginMgr().getPlugin( pluginType );
    if( plugin )
    {
        return plugin->ptopEngineRequestPluginThumb( sktBase, netIdent, thumbId, tmpThumb );
    }
    else
    {
        LogMsg( LOG_ERROR, "ThumbMgr::%s invalid plugin ", __func__ );
        vx_assert( false );
        return false;
    }

    return false;
}

//============================================================================
bool ThumbMgr::requestThumbs( std::shared_ptr<VxSktBase>& sktBase, VxNetIdent* netIdent )
{
    if( !sktBase || !netIdent || netIdent->isIgnored() )
    {
        LogMsg( LOG_ERROR, "ThumbMgr::requestThumbs invalid param " );
        return false;
    }

    for( int i = eHostTypeNetwork; i < eMaxHostType; i++ )
    {
        EHostType hostType = ( EHostType )i;
        if( netIdent->hasThumbId( hostType ) )
        {
            VxGUID thumbId = netIdent->getHostThumbId( hostType, false );
            if( !isEmoticonThumbnail( thumbId ) )
            {
                int64_t thumbTimestamp = netIdent->getHostOrThumbModifiedTime( hostType );
                if( thumbId.isValid() && thumbTimestamp && !isThumbUpToDate( thumbId, thumbTimestamp ) )
                {
                    EPluginType pluginType = HostTypeToClientPlugin( hostType );
                    if(LogEnabled(eLogPkt)) LogModule( eLogPkt,  LOG_VERBOSE, "ThumbMgr::requestThumb %s from %s %s ",
                               thumbId.toHexString().c_str(), netIdent->getOnlineName(), netIdent->getMyOnlineId().toOnlineIdString().c_str() );
                    ptopEngineRequestPluginThumb( sktBase, netIdent, pluginType, thumbId );
                }
            }
        }
    }
   
    return true;
}

//============================================================================
bool ThumbMgr::isThumbUpToDate( VxGUID& thumbId, int64_t thumbModifiedTime )
{
    bool isEmoteThumb = isEmoticonThumbnail( thumbId );

    m_ThumbInfoMutex.lock();
    for( AssetBaseInfo* thumbInfo : m_ThumbInfoList )
    {
        if( thumbInfo->getThumbId() == thumbId && ( isEmoteThumb || thumbModifiedTime <= thumbInfo->getInfoModifiedTime() ) &&
               ( isEmoteThumb || thumbInfo->isValidFile() ) )
        {
            m_ThumbInfoMutex.unlock();
            return true;
        }
    }

    m_ThumbInfoMutex.unlock();
    return false;
}

//============================================================================
uint64_t ThumbMgr::fromGuiClearCache( ECacheType cacheType )
{
    uint64_t cacheDeletedAmt{ 0 };
    if( eCacheTypeThumbnail == cacheType )
    {
        // delete every thumbnail not in use by my identity or plugin
        std::vector<VxGUID> inUseList;
        for( int i = eHostTypeNetwork; i < eMaxHostType; i++ )
        {
            VxGUID thumbId = m_Engine.getMyPktAnnounce().getHostThumbId( ( EHostType )i, false );
            if( thumbId.isValid() )
            {
                inUseList.emplace_back( thumbId );
            }
        }

        AssetBaseInfoDb& assetDb = getAssetInfoDb();
        m_ThumbInfoMutex.lock();
        for( auto iter = m_ThumbInfoList.begin(); iter != m_ThumbInfoList.end(); )
        {
            AssetBaseInfo* assetInfo = ( *iter );
            if( inUseList.end() == std::find( inUseList.begin(), inUseList.end(), assetInfo->getAssetUniqueId() ) )
            {
                assetDb.removeAsset( assetInfo );
                cacheDeletedAmt += VxFileUtil::fileExists( assetInfo->getAssetNameAndPath().c_str() );
                VxFileUtil::deleteFile( assetInfo->getAssetNameAndPath().c_str() );
                iter = m_ThumbInfoList.erase( iter );
            }
            else
            {
                ++iter;
            }
        }

        m_ThumbInfoMutex.unlock();

        // TODO clean up any not in database but the file exists?
    }

    return cacheDeletedAmt;
}

//============================================================================
void ThumbMgr::deleteThumb( VxGUID& thumbId )
{
    AssetBaseInfoDb& assetDb = getAssetInfoDb();
    m_ThumbInfoMutex.lock();
    for( auto iter = m_ThumbInfoList.begin(); iter != m_ThumbInfoList.end(); )
    {
        AssetBaseInfo* assetInfo = ( *iter );
        if( thumbId == assetInfo->getAssetUniqueId() )
        {
            announceAssetRemoved( assetInfo );
            assetDb.removeAsset( assetInfo );
            VxFileUtil::deleteFile( assetInfo->getAssetNameAndPath().c_str() );
            iter = m_ThumbInfoList.erase( iter );
            break;
        }
        else
        {
            ++iter;
        }
    }

    m_ThumbInfoMutex.unlock();
}

//============================================================================
void ThumbMgr::queryMediaThumbIfNeeded( std::shared_ptr<VxSktBase>& sktBase, VxGUID& srcOnlineId, EPluginType pluginType, VxGUID& thumbId )
{
    int64_t thumbModifiedTime{ 0 };
    if( lookupThumbInfo( thumbId, thumbModifiedTime ) )
    {
        return;
    }

    VxNetIdent* netIdent = m_Engine.getBigListMgr().findNetIdent( srcOnlineId );
    if( !netIdent )
    {
        if(LogEnabled(eLogThumbnail))LogModule( eLogThumbnail, LOG_ERROR, "ThumbMgr::%s could not lookup netIdent", __func__ );
        return;
    }

    ptopEngineRequestPluginThumb( sktBase, netIdent, pluginType, thumbId, true );
}
