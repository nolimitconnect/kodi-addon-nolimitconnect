//============================================================================
// Copyright (C) 2015 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "AssetBaseMgr.h"
#include "AssetBaseInfo.h"
#include "AssetBaseInfoDb.h"
#include "AssetBaseCallbackInterface.h"

#include <P2PEngine/P2PEngine.h>

#include <AssetMgr/AssetInfoDb.h>
#include <BlobXferMgr/BlobInfoDb.h>
#include <ThumbMgr/ThumbInfoDb.h>
#include <Plugins/FileInfo.h>
#include <Plugins/PluginFileShareServer.h>
#include <SendQueue/SendQueueMgr.h>

#include <GuiInterface/IToGui.h>

#include <PktLib/PktAnnounce.h>
#include <PktLib/PktsFileList.h>

#include <CoreLib/Sha1GeneratorMgr.h>
#include <CoreLib/VxDebug.h>
#include <CoreLib/VxFileIsTypeFunctions.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxGlobals.h>
#include <CoreLib/VxTime.h>

#include <algorithm>
#include <time.h>

namespace
{
	const char* ASSET_INFO_DB_NAME = "AssetBaseInfoDb.db3";

	//============================================================================
    static void * AssetBaseMgrStartupThreadFunc( void * pvContext )
	{
		VxThread* poThread = (VxThread*)pvContext;
		poThread->setIsThreadRunning( true );
		AssetBaseMgr * poMgr = (AssetBaseMgr *)poThread->getThreadUserParam();
        if( poMgr )
        {
            poMgr->assetInfoMgrStartup( poThread );
        }

		poThread->threadAboutToExit();
        return nullptr;
	}


	//============================================================================
	static void* AssetBaseMgrHistoryListThreadFunc( void* pvContext )
	{
		VxThread* poThread = (VxThread*)pvContext;
		poThread->setIsThreadRunning( true );
		AssetBaseMgr* poMgr = (AssetBaseMgr*)poThread->getThreadUserParam();
		if( poMgr )
		{
			poMgr->sendHistoryAssetsToGuiByThread( poThread );
		}

		poThread->threadAboutToExit();
		return nullptr;
	}
}

std::vector<VxGUID>	AssetBaseMgr::m_EmoticonIdList{
{ 3913462368200503545U, 2760340527898317750U },	  // 1 !364F694A1A7330F9264EB315D07E53B6! // emoj1.svg thumbs up
{ 13999558228189016709U, 7413105485242018473U },  // 2 !C2486C09239A728566E0A4299A59AAA9! // emoj2.svg thumbs down
{ 10829822772292086897U, 16991265692937849009U }, // 3 !964B426EBAE7EC71EBCD1A1FC7D788B1! // emoj3.svg smile
{ 7945445069844048192U, 12229757484046445461U },  // 4 !6E43E431BA6EE940A9B8D3A2BDC36B95! // emoj4.svg sun glasses
{ 2324257314457588032U, 17711723986212541605U },  // 5 !20416BB68AD91140F5CCAF1FEDF6D4A5! // emoj5.svg angel
{ 10756322002679464500U, 16760991902078506408U }, // 6 !954621DF3B6D8234E89B0154D69695A8! // emoj6.svg devil
{ 16664578291555380852U, 7906860280759028378U },  // 7 !E74479A9D7D82E746DBACF8873498A9A! // emoj7.svg heart eyes
{ 15874765662471761056U, 5995629303460670394U },  // 8 !DC4E7F430C32ACA05334C115D5B22BBA! // emoj8.svg kiss
{ 14217291826520502406U, 3076774834097433248U },  // 9 !C54DF796FBD1B0862AB2E669003D2AA0! // emoj9.svg flowers
{ 12701126956946010700U, 1186903973967391369U },  // 10 !B04377AC83E7B24C1078BAE036D36A89! // emoj10.svg heart
{ 8016853232088298773U, 14460858311392523710U },  // 11 !6F41958A1B10B515C8AF4A046DCE75BE! // emoj11.svg swear
{ 16591719547199697346U, 4982584807328050075U },  // 12 !E641A1057FBE31C24525B27BC00BEB9B! // emoj12.svg mad
{ 10037911910613534570U, 16761057474731904642U }, // 13 !8B4DD3B316C8476AE89B3CF8294D0E82! // emoj13.svg snort
{ 4489243676149697357U, 2172841312657848993U },   // 14 !3E4CFF56A3124B4D1E277BA96243BAA1! // emoj14.svg nuclear
{ 15799846624809938546U, 1468224797688694922U },  // 15 !DB4454CB49736E7214602EACACA7888A! // emoj15.svg cry flood
{ 13352407061259212155U, 13577725499120930184U }, // 16 !B94D477A66C5D57BBC6DC56750DDCD88! // emoj16.svg poop
{ 1462663878651414172U, 9453108208298294703U },   // 17 !144C6D0C50D0069C83302FD4AAF2FDAF! // emoj17.svg blue
{ 6649443090428064893U, 14264198717685600396U },  // 18 !5C479142AF23F47DC5F49D28A53B908C! // emoj18.svg happy cry
{ 2469078113775934304U, 13112475985264128399U },  // 19 !2243ED76764E5B60B5F8DF6F8B82318F! // emoj19.svg ebrows furrowed
{ 7730781286067177641U, 3789408903274709648U },   // 20 !6B49409EC84588A93496AF5204644290! // emoj20.svg suprise
{ 16159875440403194858U, 7145204477185456825U },  // 21 !E043691827CAA3EA6328DD9D762B7AB9! // emoj21.svg huh
{ 14721770146071603619U, 3089010263141508768U },  // 22 !CC4E3BF2D58C69A32ADE5E77D215E6A0! // emoj22.svg barf
{ 7227605314620074266U, 2103507223893740458U },   // 23 !644D9CBD7AA7A91A1D3128ACDEA78BAA! // emoj23.svg ok with fingers
{ 14357759964851208390U, 13198575573570798776U }, // 24 !C741029E0B4800C6B72AC28E1BD5C4B8! // emoj24.svg bandage
{ 10611472365506944914U, 2062838987330950796U },  // 25 !934385E4F9EA5F921CA0AD212CB0168C! // emoj25.svg thermometer
{ 10899107697234873459U, 4802095315587358606U },  // 26 !974168B466690C7342A4783F3726DF8E! // emoj26.svg sick
{ 1891901626467743579U, 13846726454066888592U },  // 27 !1A4162816B97F75BC0297458CCA93F90! // emoj27.svg sneeze
{ 15655074862930481624U, 16561591255687549347U }, // 28 !D941FFA4C91E51D8E5D6977FFE78D1A3! // emoj28.svg corner of mouth up
{ 15654901279462188462U, 3057683189972498358U },  // 29 !D94161C53C26E5AE2A6F12A935FE77B6! // emoj29.svg look up
{ 5207910658819667909U, 8685100309678688915U },   // 30 !48463726C8EAB7C57887ACB74D088A93! // emoj30.svg blush
{ 14574170544256737769U, 7416743895397619600U },  // 31 !CA41DAE48A6261E966ED9146FB38D790! // emoj31.svg suprise
{ 596537419207726873U, 17631004771266123946U },   // 32 !0847539F471AD719F4ADE96D2888D8AA! // emoj32.svg shush
{ 12848138990292053852U, 9367519687061681027U },  // 33 !B24DC256DD342F5C82001D862DCA3383! // emoj33.svg monical
{ 3837853253425696308U, 1553238764248287121U },   // 34 !3542CB333E840E34158E366D3A62C791! // emoj34.svg hand on chin (thinking)
{ 3910185950057618989U, 7631546539539877510U },   // 35 !3643C5678EE85E2D69E8B324E022CE86! // emoj35.svg pinocheo
{ 15657202536165776185U, 17647356409875005320U }, // 36 !D9498EC050819B39F4E80127B34FFF88! // emoj36.svg hand cover mouth
{ 238173133304132614U, 4864860177195122085U },    // 37 !034E2933EDA274064383748E57A8BDA5! // emoj37.svg hands out
{ 12341615954129516697U, 2738267793027437699U },  // 38 !AB463A52542EF8992600480CBF4B1483! // emoj38.svg nerd
{ 12632410285504908102U, 4294383552542830217U },  // 39 !AF4F5639AC82CB463B98B716A2986E89! // emoj39.svg clown
{ 15438663251370106147U, 5292085521134278792U },  // 40 !D641266DF8CADD23497143BE28325888! // emoj40.svg drool
{ 4993020490155633777U, 11738686233263127465U },  // 41 !454AC5AE7FF92C71A2E830EF576ABBA9! // emoj41.svg (tongue out raspberry)
{ 16522474626922221383U, 15482595468947255479U }, // 42 !E54B9F22241A8F47D6DD3A8C77AC70B7! // emoj42.svg( linux penguin )
{ 14504884163639376415U, 13718914074721043350U }, // 43 !C94BB34BF1E9FA1FBE635FAA3FDAC396! // emoj43.svg( raspberry pi penquin )
{ 7584589277108821538U, 10124185894024794536U },  // 44 !6941DFC33D4EDE228C80556E2D5E51A8! // emoj44.svg( ubuntu )
{ 5711226150392763480U, 8497943599727683497U },   // 45 !4F4259ECA615145875EEC2AE0F12A7A9! // emoj45.svg( linux mint )
{ 9097487941835145970U, 4367213554728824753U },   // 46 !7E40C515217BC2F23C9B7596EBE1EFB1! // emoj46.svg( freebsd )
{ 9100097679121650618U, 4285115141208480959U },   // 47 !7E4A0A9FF16713BA3B77C98475611CBF! // emoj47.svg( android )
{ 10972345742477311342U, 7384896862811218842U },  // 48 !98459A519D213D6E667C6C91E2C2639A! // emoj48.svg( windows ladybug )
{ 17456372216474597834U, 10723652770054697090U }, // 49 !F2417E0B05F779CA94D21160C5BE7082! // emoj49.svg( apple )
{ 15802608836277822000U, 16924341092067752867U }, // 50 !DB4E2502C2C9E230EADF568DE23EBBA3! // emoj50.svg( old computer )
}; 

//============================================================================
AssetBaseMgr::AssetBaseMgr( P2PEngine& engine, const char* dbName, const char* dbStateName, EAssetMgrType assetMgrType )
: m_Engine( engine )
, m_AssetMgrType( assetMgrType )
, m_AssetBaseInfoDb( createAssetInfoDb( dbName, assetMgrType ) )
, m_FileShredder( GetVxFileShredder() )
{
}

//============================================================================
AssetBaseMgr::~AssetBaseMgr()
{
	delete &m_AssetBaseInfoDb;
}

//============================================================================
bool AssetBaseMgr::isEmoticonThumbnail( VxGUID& thumbId )
{
	return thumbId.isValid() && std::find( m_EmoticonIdList.begin(), m_EmoticonIdList.end(), thumbId ) != m_EmoticonIdList.end();
}

//============================================================================
AssetBaseInfoDb& AssetBaseMgr::createAssetInfoDb( const char* dbName, EAssetMgrType assetMgrType )
{
    switch( assetMgrType )
    {
    case eAssetMgrTypeAssets:
        return *(new AssetInfoDb( *this, dbName ));
    case eAssetMgrTypeBlob:
        return *(new BlobInfoDb( *this, dbName ));
    case eAssetMgrTypeThumb:
        return *(new ThumbInfoDb( *this, dbName ));

    default:
        return *(new AssetInfoDb( *this, dbName ));
    }
}

//============================================================================
void AssetBaseMgr::onPluginsInitialized( void )
{
	if( !m_Initialized )
	{
		m_Initialized = true;
		m_AssetMgrStartupThread.startThread( (VX_THREAD_FUNCTION_T)AssetBaseMgrStartupThreadFunc, this, "AssetBaseMgrStartup" );			
	}
}

//============================================================================
void AssetBaseMgr::assetInfoMgrStartup( VxThread* startupThread )
{
	if( startupThread->isAborted() )
		return;
	// user specific directory should be set
	std::string dbName = VxGetSettingsDirectory();
	dbName += ASSET_INFO_DB_NAME; 
	lockResources();
	m_AssetBaseInfoDb.dbShutdown();
	m_AssetBaseInfoDb.dbStartup( 1, dbName );
	unlockResources();
	if( startupThread->isAborted() )
		return;
	updateAssetListFromDb( startupThread );
	m_AssetBaseListInitialized = true;
}

//============================================================================
void AssetBaseMgr::assetInfoMgrShutdown( void )
{
	m_AssetMgrStartupThread.abortThreadRun( true );
	lockResources();
	clearAssetInfoList();
	clearAssetFileListPackets();
	m_AssetBaseInfoDb.dbShutdown();
	unlockResources();
	m_AssetBaseListInitialized = false;
	m_Initialized = false;
}

//============================================================================
bool AssetBaseMgr::doesAssetExist( AssetBaseInfo& assetInfo ) // check if file still exists in directory or database
{
    if( assetInfo.isDeleted() )
    {
        return false;
    }

    if( assetInfo.isFileAsset() || assetInfo.isThumbAsset() )
    {
        if( !assetInfo.getAssetLength() || !( assetInfo.getAssetLength() == (int64_t)VxFileUtil::getFileLen( assetInfo.getAssetNameAndPath().c_str() ) ) )
        {
            LogMsg( LOG_WARN, "File %s no longer exists for asset %s length %lld", assetInfo.getAssetNameAndPath().c_str(), assetInfo.getAssetUniqueId().toOnlineIdString().c_str(), assetInfo.getAssetLength() );
            assetInfo.setIsDeleted( true );
            updateDatabase( &assetInfo );
            return false;
        }

        return true;
    }

    // TODO verify exists in database
    return true;
}


//============================================================================
bool AssetBaseMgr::getAsset( std::string& fileNameAndPath, AssetBaseInfo& assetInfo )
{
	for( auto asset : m_AssetBaseInfoList )
	{
		if( asset->getAssetNameAndPath() == fileNameAndPath )
		{
			assetInfo = *asset;
			return true;
		}
	}

	return false;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::findAsset( std::string& fileNameAndPath )
{
	std::vector<AssetBaseInfo*>::iterator iter;
	for( iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
        if( (*iter)->getAssetNameAndPath() == fileNameAndPath )
		{
			return (*iter);
		}
	}

	return 0;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::findAsset( VxSha1Hash& fileHashId )
{
	if( false == fileHashId.isHashValid() )
	{
		LogMsg( LOG_ERROR, "AssetBaseMgr::findAsset: invalid file hash id" );
		return 0;
	}

	std::vector<AssetBaseInfo*>::iterator iter;
	for( iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
		if( (*iter)->getAssetHashId() == fileHashId )
		{
			return (*iter);
		}
	}

	return 0;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::findAsset( VxGUID& assetId )
{
	if( false == assetId.isValid() )
	{
		//LogMsg( LOG_ERROR, "AssetBaseMgr::findAsset: invalid VxGUID asset id\n" );
        return nullptr;
	}

	for( AssetBaseInfo* assetInfo : m_AssetBaseInfoList )
	{
		if( assetInfo->getAssetUniqueId() == assetId )
		{
			return assetInfo;
		}
	}

	return nullptr;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::addAssetFile( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen )
{
	VxGUID assetId;
	assetId.initializeWithNewVxGUID();
    return addAssetFile( assetType, fileName, fileNameAndPath, fileLen, assetId );
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::addAssetFile( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId )
{
    AssetBaseInfo* assetInfo = createAssetInfo( assetType, fileName, fileNameAndPath, fileLen, assetId );
	if( assetInfo )
	{
		if( !assetInfo->isValid() )
		{
			return nullptr;
		}

		if( insertNewInfo( assetInfo ) )
		{
			return assetInfo;
		}
	}

	return nullptr;
}

//============================================================================
bool AssetBaseMgr::addAssetFile(	EAssetType      assetType,
                                    const char*		fileName, 
                                    const char*		fileNameAndPath,
									VxGUID&			assetId,  
									uint8_t *		hashId, 
									EAssetLocation	locationFlags, 
									const char*		assetTag, 
                                    int64_t			timestamp )
{
    AssetBaseInfo* assetInfo = createAssetInfo( assetType, fileName, fileNameAndPath, assetId, hashId, locationFlags, assetTag, timestamp );
	if( assetInfo )
	{
		if( !assetInfo->isValid() )
		{
            return false;
		}

		return insertNewInfo( assetInfo );
	}
	
	return false;
}

//============================================================================
bool AssetBaseMgr::addAssetFile(	EAssetType      assetType,
                                    const char*		fileName, 
                                    const char*		fileNameAndPath,
									VxGUID&			assetId,  
									VxGUID&		    creatorId, 
									VxGUID&		    historyId, 
									uint8_t *		hashId, 
									EAssetLocation	locationFlags, 
									const char*		assetTag, 
                                    int64_t			timestamp )
{
    AssetBaseInfo* assetInfo = createAssetInfo( assetType, fileName, fileNameAndPath, assetId, hashId, locationFlags, assetTag, timestamp );
	if( assetInfo )
	{
		assetInfo->setCreatorId( creatorId );
		assetInfo->setHistoryId( historyId );

		if( !assetInfo->isValid() )
		{
            return false;
		}

		return insertNewInfo( assetInfo );
	}
	
	return false;
}

//============================================================================
bool AssetBaseMgr::addAsset( AssetBaseInfo& assetInfo, AssetBaseInfo*& retCreatedAsset )
{
	if( !assetInfo.isValid() )
	{
		return false;
	}

	AssetBaseInfo* newAssetBaseInfo = createAssetInfo( assetInfo );
	LogMsg( LOG_VERBOSE, "AssetBaseMgr::addAsset" );

	if( newAssetBaseInfo != nullptr && insertNewInfo( newAssetBaseInfo ) )
	{
		retCreatedAsset = newAssetBaseInfo;
		return true;
	}
	else
	{
		return false;
	}
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::createAssetInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen )
{
    AssetBaseInfo* assetInfo = new AssetBaseInfo( assetType, fileName, fileNameAndPath, fileLen );
    if( assetInfo )
    {
        assetInfo->getAssetUniqueId().initializeWithNewVxGUID();
    }

	assetInfo->assureHasCreatorId();
    return assetInfo;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::createAssetInfo( EAssetType assetType, const char* fileName, const char* fileNameAndPath, uint64_t fileLen, VxGUID& assetId )
{
    AssetBaseInfo* assetInfo = new AssetBaseInfo( assetType, fileName, fileNameAndPath, fileLen, assetId );
	assetInfo->assureHasCreatorId();
	return assetInfo;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::createAssetInfo( FileInfo& fileInfo )
{
	AssetBaseInfo* assetInfo = new AssetBaseInfo( fileInfo );
	assetInfo->assureHasCreatorId();
	return assetInfo;
}

//============================================================================
AssetBaseInfo* AssetBaseMgr::createAssetInfo( 	EAssetType      assetType, 
                                                const char*		fileName, 
                                                const char*     fileNameAndPath,
										        VxGUID&			assetId,  
										        uint8_t *	    hashId, 
										        EAssetLocation	locationFlags, 
										        const char*		assetTag, 
                                                int64_t			timestamp )
{
    uint64_t  fileLen = VxFileUtil::getFileLen( fileNameAndPath );
	if( ( false == isAllowedFileOrDir( fileName ) )
		|| ( 0 == fileLen ) )
	{
		LogMsg( LOG_ERROR, "ERROR %d AssetBaseMgr::createAssetInfo could not get file info %s", VxGetLastError(), fileName );
		return NULL;
	}

    AssetBaseInfo* assetInfo = createAssetInfo( assetType, fileName, fileNameAndPath, fileLen );
	assetInfo->setAssetUniqueId( assetId );
	if( false == assetInfo->getAssetUniqueId().isValid() )
	{
		assetInfo->getAssetUniqueId().initializeWithNewVxGUID();
	}

	assetInfo->getAssetHashId().setHashData( hashId );
	assetInfo->setLocationFlags( locationFlags );
	assetInfo->setAssetTag( assetTag );
	assetInfo->setCreationTime( timestamp ? timestamp : GetTimeStampMs() );
	assetInfo->assureHasCreatorId();

	return assetInfo;
}

//============================================================================
bool AssetBaseMgr::insertNewInfo( AssetBaseInfo* assetInfo )
{
	if( !assetInfo->isValid() )
	{
		return false;
	}

	AssetBaseInfo* assetInfoExisting = findAsset( assetInfo->getAssetUniqueId() );
	if( assetInfoExisting )
	{
		LogMsg( LOG_ERROR, "ERROR AssetBaseMgr::insertNewInfo: duplicate assset %s", assetInfo->getAssetName().c_str() );
		if( assetInfoExisting != assetInfo )
		{
            // update existing asset
			*assetInfoExisting = *assetInfo;
		}
	}

	if( 0 == assetInfo->getCreationTime() )
	{
		assetInfo->setCreationTime( GetTimeStampMs() );
	}

    updateDatabase( assetInfo );
	if( !assetInfoExisting )
	{
		lockResources();
		m_AssetBaseInfoList.emplace_back( assetInfo );
		unlockResources();
		announceAssetAdded( assetInfo );
	}
    else
    {
        announceAssetUpdated( assetInfo );
    }

	return true;
}

//============================================================================
bool AssetBaseMgr::updateAsset( AssetBaseInfo& assetInfo )
{
	if( !assetInfo.isValid() )
	{
		return false;
	}

    AssetBaseInfo* existingAsset = findAsset( assetInfo.getAssetUniqueId() );
    if( existingAsset )
    {
        *existingAsset = assetInfo;
        updateDatabase( existingAsset );
        announceAssetUpdated( existingAsset );
        return true;
    }

    return false;
}

//============================================================================
bool AssetBaseMgr::updateAsset( FileInfo& fileInfo )
{
	AssetBaseInfo* existingAsset = findAsset( fileInfo.getAssetId() );
    if( existingAsset )
    {
		bool updateDb{ false };
		if( !existingAsset->isInLibrary() && fileInfo.getIsInLibrary() )
		{
			existingAsset->setIsInLibrary( fileInfo.getIsInLibrary() );
			updateDb = true;
		}

		if( !existingAsset->isSharedFileAsset() && fileInfo.getIsSharedFile() )
		{
			existingAsset->setIsSharedFileAsset( fileInfo.getIsSharedFile() );
			updateDb = true;
		}

		if( updateDb )
		{
			updateDatabase( existingAsset );
		}
    }
	else
	{
		AssetBaseInfo* newAssetInfo = createAssetInfo( fileInfo );
		newAssetInfo->setCreationTime( GetGmtTimeMs() );
		newAssetInfo->setModifiedTime( GetGmtTimeMs() );
		insertNewInfo( newAssetInfo );
	}

	return true;
}

//============================================================================
void AssetBaseMgr::announceAssetAdded( AssetBaseInfo* assetInfo, bool resourceLocked )
{
	if( !assetInfo->isValid() )
	{
		return;
	}

	// LogMsg( LOG_VERBOSE, "AssetBaseMgr::announceAssetAdded start" );
	if( assetInfo->isFileAsset() )
	{
		updateFileListPackets(resourceLocked);
		updateAssetFileTypes(resourceLocked);
	}
	
	lockClientList();
	std::vector<AssetBaseCallbackInterface *>::iterator iter;
	for( iter = m_AssetClients.begin();	iter != m_AssetClients.end(); ++iter )
	{
		AssetBaseCallbackInterface * client = *iter;
		client->callbackAssetAdded( assetInfo );
	}

	m_Engine.getToGui().toGuiAssetAdded( assetInfo );
	unlockClientList();
	// LogMsg( LOG_VERBOSE, "AssetBaseMgr::announceAssetAdded done" );
}

//============================================================================
void AssetBaseMgr::announceAssetUpdated( AssetBaseInfo* assetInfo )
{
	if( !assetInfo->isValid() )
	{
		return;
	}

    // LogMsg( LOG_VERBOSE, "AssetBaseMgr::announceAssetUpdated start" );
    lockClientList();
    for( auto& client : m_AssetClients )
    {
        client->callbackAssetAdded( assetInfo );
    }

	m_Engine.getToGui().toGuiAssetUpdated( assetInfo );
    unlockClientList();
    // LogMsg( LOG_VERBOSE, "AssetBaseMgr::announceAssetUpdated done" );
}

//============================================================================
void AssetBaseMgr::announceAssetRemoved( AssetBaseInfo* assetInfo, bool resourceLocked )
{
    // Mark as dirty for lazy update instead of expensive immediate rebuild
    // Only shared files affect search file types and file list packets
    if( assetInfo->isSharedFileAsset() )
    {
        markFileListPacketsDirty();
        markAssetFileTypesDirty();
    }

	lockClientList();
    for( auto& client : m_AssetClients )
    {
		client->callbackAssetRemoved( assetInfo );
	}

	m_Engine.getToGui().toGuiAssetRemoved( assetInfo );
	unlockClientList();
}

//============================================================================
void AssetBaseMgr::announceAssetXferState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState assetSendState, int param )
{
	LogMsg( LOG_VERBOSE, "AssetBaseMgr::announceAssetXferState state %d start", assetSendState );
	lockClientList();
    for( auto& client : m_AssetClients )
    {
		client->callbackAssetSendState( sendToId, assetUniqueId, assetSendState, param );
	}

	m_Engine.getToGui().toGuiAssetXferState( assetUniqueId, assetSendState, param );
	unlockClientList();
	LogMsg( LOG_VERBOSE, "AssetBaseMgr::announceAssetXferState state %d done", assetSendState );
}

//============================================================================
bool AssetBaseMgr::removeAsset( std::string fileNameAndPath, bool deleteFile )
{
	bool assetRemoved = false;
	std::vector<AssetBaseInfo*>::iterator iter;
	for( iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
        if( fileNameAndPath == (*iter)->getAssetNameAndPath() )
		{
			AssetBaseInfo* assetInfo = *iter;
			m_AssetBaseInfoList.erase( iter );
            m_AssetBaseInfoDb.removeAsset( fileNameAndPath.c_str() );
			announceAssetRemoved( assetInfo );
            if( deleteFile && ( assetInfo->isThumbAsset() || assetInfo->isFileAsset() ) )
			{
                VxFileUtil::deleteFile( assetInfo->getFileNameAndPath().c_str() );
			}

			delete assetInfo;
			assetRemoved = true;
			break;
		}
	}

	return assetRemoved;
}

//============================================================================
bool AssetBaseMgr::removeAsset( VxGUID& assetUniqueId, bool deleteFile )
{
	m_Engine.getSendQueueMgr().removeAsset( assetUniqueId );
	bool assetRemoved = false;
	std::vector<AssetBaseInfo*>::iterator iter;
	for ( iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
		if( assetUniqueId == ( *iter )->getAssetUniqueId() )
		{
			AssetBaseInfo* assetInfo = *iter;
            std::string fileName = assetInfo->getAssetNameAndPath();
			m_AssetBaseInfoList.erase( iter );
			m_AssetBaseInfoDb.removeAsset( assetInfo );
			announceAssetRemoved( assetInfo );
            if( deleteFile && ( assetInfo->isThumbAsset() || assetInfo->isFileAsset() ) )
			{
				GetVxFileShredder().shredFile( fileName );
			}

			delete assetInfo;
			assetRemoved = true;
			break;
		}
	}

	return assetRemoved;
}

//============================================================================
void AssetBaseMgr::clearAssetInfoList( void )
{
	for( auto iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
		delete (*iter);
	}

	m_AssetBaseInfoList.clear();
}

//============================================================================
void AssetBaseMgr::updateAssetListFromDb( VxThread* startupThread )
{
	std::vector<AssetBaseInfo*> toDeleteList;
	lockResources();
	clearAssetInfoList();
	m_AssetBaseInfoDb.getAllAssets( m_AssetBaseInfoList );

	// there should not be any without valid hash but if is then generate it
	for( auto iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end();  )
	{
		if( startupThread->isAborted() )
		{
			unlockResources();
			return;
		}

		AssetBaseInfo* assetInfo = (*iter);
		if( !assetInfo->validateAssetExist() )
		{
			// add to list to remove from database and delete
			toDeleteList.emplace_back( assetInfo );
			++iter;
			continue;
		}

		if( !assetInfo->isValid() )
		{
			toDeleteList.emplace_back( assetInfo );
			++iter;
			continue;
		}

		if( assetInfo->isSharedFileAsset() )
		{
			m_Engine.getPluginFileShareServer().fileShareEnable( assetInfo, true );
		}

		EAssetSendState sendState = assetInfo->getAssetSendState();
		if( eAssetSendStateTxProgress == sendState ) 
		{
			assetInfo->setAssetSendState( eAssetSendStateTxFail );
			m_AssetBaseInfoDb.updateAssetSendState( assetInfo->getAssetUniqueId(), eAssetSendStateTxFail );
		}
		else if(  eAssetSendStateRxProgress == sendState  )
		{
			assetInfo->setAssetSendState( eAssetSendStateRxFail );
			m_AssetBaseInfoDb.updateAssetSendState( assetInfo->getAssetUniqueId(), eAssetSendStateRxFail );
		}

		if( assetInfo->needsHashGenerated() )
		{
			m_WaitingForHastList.emplace_back( assetInfo );
			iter = m_AssetBaseInfoList.erase( iter );
            requestFileHash( assetInfo );
		}
		else
		{
			++iter;
		}
	}

	for( auto assetInfo : toDeleteList )
	{
		m_AssetBaseInfoDb.removeAsset( assetInfo );
		if( assetInfo->isThumbAsset() || assetInfo->isFileAsset() )
		{
            GetVxFileShredder().shredFile( assetInfo->getAssetNameAndPath() );
		}
	}

	unlockResources();
	updateFileListPackets();
	updateAssetFileTypes();
}

//============================================================================
uint16_t AssetBaseMgr::getAssetBaseFileTypes( void )
{
    if( m_AssetFileTypesDirty )
    {
        updateAssetFileTypes();
    }
    return m_u16AssetBaseFileTypes;
}

//============================================================================
std::vector<PktFileListReply*>& AssetBaseMgr::getFileListPackets( void )
{
    if( m_FileListPacketsDirty )
    {
        updateFileListPackets();
    }
    return m_FileListPackets;
}

//============================================================================
void AssetBaseMgr::updateAssetFileTypes( bool resourceLocked )
{
    m_AssetFileTypesDirty = false;
	uint16_t u16FileTypes = 0;
	if( !resourceLocked )
	{
		lockResources();
	}

	for( auto assetInfo : m_AssetBaseInfoList )
	{
		if( assetInfo->isFileAsset() )
		{
			u16FileTypes |= assetInfo->getAssetType();
		}
	}

	if( !resourceLocked )
	{
		unlockResources();
	}

	// ignore extended types
	u16FileTypes = u16FileTypes & 0xff;
	if( m_u16AssetBaseFileTypes != u16FileTypes )
	{
		m_u16AssetBaseFileTypes = u16FileTypes;

		lockClientList();
		for( auto& client : m_AssetClients )
		{
			client->callbackAssetFileTypesChanged( u16FileTypes );
		}

		unlockClientList();
	}
}

//============================================================================
void AssetBaseMgr::updateFileListPackets( bool resourceLocked )
{
    m_FileListPacketsDirty = false;
	bool hadAssetBaseFiles = m_FileListPackets.size() ? true : false;
	PktFileListReply * pktFileList = 0;
	clearAssetFileListPackets();
	lockFileListPackets();
	if( !resourceLocked )
	{
		lockResources();
	}

	std::vector<AssetBaseInfo*>::iterator iter;
	for( iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
		AssetBaseInfo* assetInfo = (*iter); 
		if( ( false == assetInfo->isFileAsset() ) || ( false == assetInfo->getAssetHashId().isHashValid() ) )
			continue;

		if( 0 == pktFileList )
		{
			pktFileList = new PktFileListReply();
			pktFileList->setListIndex( (uint32_t)m_FileListPackets.size() );
		}

		if( pktFileList->canAddFile( (uint32_t)(assetInfo->getRemoteAssetName().size() + 1) ) )
		{
			pktFileList->addFile(	assetInfo->getAssetHashId(),
									assetInfo->getAssetLength(),
									assetInfo->getAssetType(),
									assetInfo->getRemoteAssetName().c_str() );
		}
		else
		{
			m_FileListPackets.emplace_back( pktFileList );
			pktFileList = new PktFileListReply();
			pktFileList->setListIndex( (uint32_t)m_FileListPackets.size() );
			pktFileList->addFile(	assetInfo->getAssetHashId(),
									assetInfo->getAssetLength(),
									assetInfo->getAssetType(),
									assetInfo->getAssetName().c_str() );
		}
	}

	if( 0 != pktFileList )
	{
		if( pktFileList->getFileCount() )
		{
			pktFileList->setIsListCompleted( true ); // last pkt in list
			m_FileListPackets.emplace_back( pktFileList );
		}
		else
		{
			delete pktFileList;
		}
	}

	if( !resourceLocked )
	{
		unlockResources();
	}

	unlockFileListPackets();
	if( hadAssetBaseFiles || m_FileListPackets.size() )
	{
		lockClientList();
		std::vector<AssetBaseCallbackInterface *>::iterator iter;
		for( iter = m_AssetClients.begin();	iter != m_AssetClients.end(); ++iter )
		{
			AssetBaseCallbackInterface * client = *iter;
			client->callbackAssetPktFileListUpdated();
		}

		unlockClientList();
	}
}

//============================================================================
void AssetBaseMgr::clearAssetFileListPackets( void )
{
	lockFileListPackets();
	std::vector<PktFileListReply*>::iterator iter;
	for( iter = m_FileListPackets.begin(); iter != m_FileListPackets.end(); ++iter )
	{
		delete (*iter);
	}

	m_FileListPackets.clear();
	unlockFileListPackets();
}

//============================================================================
bool AssetBaseMgr::fromGuiSetFileIsShared( FileInfo& fileInfo, bool shareFile )
{
	lockResources();
	AssetBaseInfo* assetInfo = findAsset( fileInfo.getAssetId() );
	if( assetInfo )
	{
		if( shareFile != assetInfo->isSharedFileAsset() )
		{
			assetInfo->setIsSharedFileAsset( shareFile );
			bool inUse = assetInfo->isMediaFileInUse();
			bool assetRemoved{ false };
			if( !shareFile && !inUse )
			{
				VxGUID thumbId;
				VxGUID assetId = assetInfo->getAssetUniqueId();
				if( assetInfo->getThumbId().isValid() )
				{
					thumbId = assetInfo->getThumbId();
					assetInfo->getThumbId().clear();
				}

				unlockResources(); // must unlock before removal or deadlocks

				removeAsset( assetId, false );

				if( thumbId.isValid() )
				{
					deleteThumbAsset( thumbId );
				}

				return true;
			}
			else
			{
				updateDatabase( assetInfo );
			}

			unlockResources();

			updateAssetFileTypes();
			updateFileListPackets();
			if( shareFile && !assetInfo->getAssetHashId().isHashValid() )
			{
				requestFileHash( assetInfo );
			}

			if( !assetRemoved )
			{
				m_Engine.getPluginFileShareServer().fileShareEnable( assetInfo, shareFile );
			}

			return true;
		}

		unlockResources();
		return true;
	}

	unlockResources();

	if( shareFile )
	{
		// file is not currently AssetBase and should be
		VxGUID guid;
		AssetBaseInfo* assetInfo = createAssetInfo( fileInfo );
		if( assetInfo )
		{
			assetInfo->setIsSharedFileAsset( true );
			insertNewInfo( assetInfo );
			FileInfo fileInfo = assetInfo->getFileInfo();
			m_Engine.getPluginFileShareServer().fromGuiSetFileIsShared( fileInfo, true );
			if( !assetInfo->getAssetHashId().isHashValid() )
			{
				requestFileHash( assetInfo );
			}

			m_Engine.getPluginFileShareServer().fileShareEnable( assetInfo, shareFile );
		}
	}
	else
	{
		return false;
	}

	return true;
}

//============================================================================
bool AssetBaseMgr::fromGuiSetFileIsShared( std::string& fileNameAndPath, bool shareFile )
{
	bool foundAsset{ false };
	FileInfo fileInfo;
	lockResources();
	AssetBaseInfo* assetInfo = findAsset( fileNameAndPath );
	if( assetInfo )
	{
		fileInfo = assetInfo->getFileInfo();
		foundAsset = true;
	}

	unlockResources();

	if( foundAsset )
	{
		return fromGuiSetFileIsShared( fileInfo, shareFile );
	}
	else
	{
		if( VxFileUtil::getFileInfo( fileNameAndPath.c_str(), fileInfo ) )
		{
			return fromGuiSetFileIsShared( fileInfo, shareFile );
		}
		else
		{
			LogMsg( LOG_ERROR, "AssetBaseMgr::%s failed file does not exist %s", __func__, fileNameAndPath.c_str() );
		}
	}

	return false;
}

//============================================================================
bool AssetBaseMgr::fromGuiGetFileIsShared( std::string& fileNameAndPath )
{
	bool isShared{ false };
	lockResources();
	AssetBaseInfo* assetInfo = findAsset( fileNameAndPath );
	if( assetInfo )
	{
		isShared = assetInfo->isSharedFileAsset();
	}

	unlockResources();
	return isShared;
}

//============================================================================
bool AssetBaseMgr::fromGuiSetFileIsInLibrary( FileInfo& fileInfo, bool isInLibrary )
{
	lockResources();
	AssetBaseInfo* assetInfo = findAsset( fileInfo.getAssetId() );
	if( assetInfo )
	{
		if( isInLibrary != assetInfo->isInLibrary() )
		{
			assetInfo->setIsInLibrary( isInLibrary );
			bool inUse = assetInfo->isMediaFileInUse();
			if( !isInLibrary && !inUse )
			{
				VxGUID thumbId;
				VxGUID assetId = assetInfo->getAssetUniqueId();
				if( assetInfo->getThumbId().isValid() )
				{
					thumbId = assetInfo->getThumbId();
					assetInfo->getThumbId().clear();
				}

				unlockResources(); // must unlock before removal or deadlocks

				removeAsset( assetId, false );

				if( thumbId.isValid() )
				{
					deleteThumbAsset( thumbId );
				}

				return true;
			}
			else
			{
				updateDatabase( assetInfo );
			}			
		}

		unlockResources();

		return true;
	}

	unlockResources();

	if( isInLibrary )
	{
		// file is not currently AssetBase and should be
		VxGUID guid;
		AssetBaseInfo* assetInfo = createAssetInfo( fileInfo );
		if( assetInfo )
		{
			assetInfo->setIsInLibrary( true );
			insertNewInfo( assetInfo );
			if( !assetInfo->getAssetHashId().isHashValid() )
			{
				requestFileHash( assetInfo );
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

//============================================================================
bool AssetBaseMgr::fromGuiSetFileIsInLibrary(  std::string& fileNameAndPath, bool isInLibrary )
{
	bool foundAsset{ false };
	FileInfo fileInfo;
	lockResources();
	AssetBaseInfo* assetInfo = findAsset( fileNameAndPath );
	if( assetInfo )
	{
		fileInfo = assetInfo->getFileInfo();
		foundAsset = true;
	}

	unlockResources();

	if( foundAsset )
	{
		return fromGuiSetFileIsInLibrary( fileInfo, isInLibrary );
	}
	else
	{
		if( VxFileUtil::getFileInfo( fileNameAndPath.c_str(), fileInfo ) )
		{
			return fromGuiSetFileIsInLibrary( fileInfo, isInLibrary );
		}
		else
		{
			LogMsg( LOG_ERROR, "AssetBaseMgr::%s failed file does not exist %s", __func__, fileNameAndPath.c_str() );
		}
	}

	return false;
}

//============================================================================
bool AssetBaseMgr::fromGuiGetFileIsInLibrary( std::string& fileNameAndPath )
{
	bool isInLibrary{ false };
	lockResources();
	AssetBaseInfo* assetInfo = findAsset( fileNameAndPath );
	if( assetInfo )
	{
		isInLibrary = assetInfo->isInLibrary();
	}

	unlockResources();
	return isInLibrary;
}

//============================================================================
void AssetBaseMgr::fromGuiSendFileList( VxGUID& appInstId, uint8_t fileTypeFilter, bool inLibrary, bool isShared )
{
	lockResources();
	for( auto assetInfo : m_AssetBaseInfoList )
	{
		if( 0 != (fileTypeFilter & assetInfo->getFileType()) )
		{
			if( inLibrary && assetInfo->isInLibrary() || isShared && assetInfo->isSharedFileAsset() )
			{
				FileInfo fileInfo = assetInfo->getFileInfo();
				IToGui::getIToGui().toGuiFileList( appInstId, fileInfo );
			}
		}
	}

	unlockResources();
	IToGui::getIToGui().toGuiFileListCompleted( appInstId );
}

//============================================================================
bool AssetBaseMgr::getFileHashId( std::string& fileFullName, VxSha1Hash& retFileHashId )
{
	bool foundHash = false;
	lockResources();
	for( auto assetInfo : m_AssetBaseInfoList )
	{
        if( fileFullName == assetInfo->getAssetNameAndPath() )
		{
			retFileHashId = assetInfo->getAssetHashId();
			foundHash = retFileHashId.isHashValid();
			break;
		}
	}

	unlockResources();
	return foundHash;
}

//============================================================================
bool AssetBaseMgr::getFileFullName( VxSha1Hash& fileHashId, std::string& retFileFullName )
{
	bool isAssetBase = false;
	lockResources();
	std::vector<AssetBaseInfo*>::iterator iter;
	for( iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
		if( fileHashId == (*iter)->getAssetHashId() )
		{
			isAssetBase = true;
            retFileFullName = (*iter)->getAssetNameAndPath();
			break;
		}
	}

	unlockResources();
	return isAssetBase;
}

//============================================================================
void AssetBaseMgr::updateDatabase( AssetBaseInfo* assetInfo )
{
	if( !assetInfo->isValid() )
	{
		return;
	}

	m_AssetBaseInfoDb.addAsset( assetInfo );
}

//============================================================================
void AssetBaseMgr::updateAssetDatabaseSendState( VxGUID& assetUniqueId, enum EAssetSendState sendState )
{
	m_AssetBaseInfoDb.updateAssetSendState( assetUniqueId, sendState );
}

//============================================================================
void AssetBaseMgr::fromGuiQuerySessionHistory( GroupieId& groupieId )
{
	// if we send all now while in the gui thread we may overflow the stack because qt cannot process while in this call
	// instead spin up a thread to send the assets
	lockResources();
	m_HistorySendList.emplace_back( groupieId );
	unlockResources();

	if( !m_HistoryListThread.isThreadRunning() )
	{
		m_HistoryListThread.startThread( (VX_THREAD_FUNCTION_T)AssetBaseMgrHistoryListThreadFunc, this, "AssetBaseMgrHistoryList" );
	}	
}

//============================================================================
void AssetBaseMgr::sendHistoryAssetsToGuiByThread( VxThread* poThread )
{
	while( !m_HistorySendList.empty() && !VxIsAppShuttingDown() )
	{
		lockResources();
		if( m_HistorySendList.empty() )
		{
			unlockResources();
			break;
		}

		GroupieId groupieId = m_HistorySendList.front();
		m_HistorySendList.erase( m_HistorySendList.begin() );
		if( groupieId.getHostedId().isValid() )
		{
			for( auto assetInfo : m_AssetBaseInfoList )
			{
				if( assetInfo->isHistoryMatch( groupieId ) )
				{
					onQueryHistoryAsset( assetInfo );				
				}
			}
		}

		unlockResources();
	}
}

//============================================================================
void AssetBaseMgr::updateAssetXferState( VxGUID& sendToId, VxGUID& assetUniqueId, EAssetSendState assetSendState, int param )
{
	switch( assetSendState )
	{
	case eAssetSendStateRxFail:
	case eAssetSendStateTxFail:
	case eAssetSendStateTxPermissionErr:
	case eAssetSendStateRxPermissionErr:
		LogMsg( LOG_VERBOSE, "AssetBaseMgr::updateAssetXferState FAILED %s state %s", assetUniqueId.toOnlineIdString().c_str(), DescribeAssetSendState( assetSendState ) );
		break;

	default:
		LogMsg( LOG_VERBOSE, "AssetBaseMgr::updateAssetXferState %s state %s", assetUniqueId.toOnlineIdString().c_str(), DescribeAssetSendState( assetSendState ) );
	}

	bool assetSendStateChanged{ false };
	bool assetSendStateFound{ false };
	lockResources();
	for( auto assetInfo : m_AssetBaseInfoList )
	{
		if( assetInfo->getAssetUniqueId() == assetUniqueId )
		{
			assetSendStateFound = true;
			EAssetSendState oldSendState = assetInfo->getAssetSendState();
			if( oldSendState != assetSendState || 
				( eAssetSendStateTxProgress == assetSendState || eAssetSendStateRxProgress == assetSendState ) )
			{
				assetInfo->setAssetSendState( assetSendState );
				assetSendStateChanged = true;

				updateAssetDatabaseSendState( assetUniqueId, assetSendState );
				switch( assetSendState )
				{
				case eAssetSendStateTxProgress:
					if( eAssetSendStateNone == oldSendState )
					{
						IToGui::getIToGui().toGuiAssetAction( eAssetActionTxBegin, assetUniqueId, param );
					}

					IToGui::getIToGui().toGuiAssetAction( eAssetActionTxProgress, assetUniqueId, param );
					break;

				case eAssetSendStateRxProgress:
					if( eAssetSendStateNone == oldSendState )
					{
						IToGui::getIToGui().toGuiAssetAction( eAssetActionRxBegin, assetUniqueId, param );
					}

					IToGui::getIToGui().toGuiAssetAction( eAssetActionRxProgress, assetUniqueId, param );
					break;

				case eAssetSendStateRxSuccess:
					if( ( eAssetSendStateNone == oldSendState )
						|| ( eAssetSendStateRxProgress == oldSendState ) )
					{
						IToGui::getIToGui().toGuiAssetAction( eAssetActionRxSuccess, assetUniqueId, param );
						IToGui::getIToGui().toGuiAssetAction( eAssetActionRxNotifyNewMsg, assetInfo->getCreatorId(), 100 );
					}
					else 
					{
						IToGui::getIToGui().toGuiAssetAction( eAssetActionRxSuccess, assetUniqueId, param );
					}

					break;

				case eAssetSendStateTxSuccess:
					IToGui::getIToGui().toGuiAssetAction( eAssetActionTxSuccess, assetUniqueId, param );
					break;

				case eAssetSendStateRxFail:
					IToGui::getIToGui().toGuiAssetAction( eAssetActionRxError, assetUniqueId, param );
					break;

				case eAssetSendStateTxFail:
					IToGui::getIToGui().toGuiAssetAction( eAssetActionTxError, assetUniqueId, param );
					break;

				case eAssetSendStateTxPermissionErr:
					IToGui::getIToGui().toGuiAssetAction( eAssetActionTxError, assetUniqueId, param );
					break;

				case eAssetSendStateRxPermissionErr:
					IToGui::getIToGui().toGuiAssetAction( eAssetActionRxError, assetUniqueId, param );
					break;


				case eAssetSendStateNone:
				default:
					break;
				}
			}

			break;
		}
	}

	if( !assetSendStateFound )
	{
		LogMsg( LOG_ERROR, "AssetBaseMgr::updateAssetXferState asset not found" );
	}

	unlockResources();
	if( assetSendStateChanged )
	{
		announceAssetXferState( sendToId, assetUniqueId, assetSendState, param );
	}

	LogMsg( LOG_VERBOSE, "AssetBaseMgr::updateAssetXferState state %d done", assetSendState );
}

//============================================================================
bool AssetBaseMgr::isAllowedFileOrDir( std::string strFileName )
{
    if( VxIsExecutableFile( strFileName ) 
        || VxIsShortcutFile( strFileName ) )
    {
        return false;
    }

    return true;
}

//============================================================================
void AssetBaseMgr::addAssetMgrClient( AssetBaseCallbackInterface * client, bool enable )
{
    AutoResourceLock( this );
    if( enable )
    {
        m_AssetClients.emplace_back( client );
    }
    else
    {
        std::vector<AssetBaseCallbackInterface *>::iterator iter;
        for( iter = m_AssetClients.begin(); iter != m_AssetClients.end(); ++iter )
        {
            if( *iter == client )
            {
                m_AssetClients.erase( iter );
                return;
            }
        }
    }
}

//============================================================================
void AssetBaseMgr::onQueryHistoryAsset( AssetBaseInfo* assetInfo )
{
	m_Engine.getToGui().toGuiAssetSessionHistory( assetInfo );
}

//============================================================================
bool AssetBaseMgr::fromGuiQueryFileHash( FileInfo& fileInfo )
{
	bool result{ false };
	lockResources();
	for( auto* assetInfo : m_AssetBaseInfoList )
	{
        if( fileInfo.getFileLength() == assetInfo->getAssetLength() && fileInfo.getFileNameAndPath() == assetInfo->getAssetNameAndPath() )
		{
			if( assetInfo->getAssetHashId().isHashValid() )
			{
				fileInfo.setFileHashId( assetInfo->getAssetHashId() );
				result = true;
			}

			break;
		}
	}

	unlockResources();
	return result;
}

//============================================================================
void AssetBaseMgr::getStreamableAssets( std::vector<AssetBaseInfo>& streamableAssets )
{
	streamableAssets.clear();

	lockResources();
	for( auto* assetInfo : m_AssetBaseInfoList )
	{
        if( assetInfo->getIsAssetStreamable() && assetInfo->isSharedFileAsset() )
		{
			streamableAssets.emplace_back( *assetInfo );
		}
	}

	unlockResources();
}

//============================================================================
void AssetBaseMgr::getSharedFiles( std::vector<AssetBaseInfo>& sharedFiles )
{
	sharedFiles.clear();
	int assetIdx{ 0 };

	lockResources();
	for( auto* assetInfo : m_AssetBaseInfoList )
	{
		if( assetInfo->isPhotoAsset() || assetInfo->isVideoAsset() || assetInfo->isAudioAsset() )
		{
			if( assetInfo->isSharedFileAsset() )
			{
				sharedFiles.emplace_back( *assetInfo );
				assetIdx++;
			}
		}
	}

	LogMsg( LOG_VERBOSE, "%s %d of %zu assets are shared", __func__, assetIdx, m_AssetBaseInfoList.size() );

	unlockResources();
}

//============================================================================
void AssetBaseMgr::deleteFile( std::string fileNameAndPath, bool shredFile )
{
	// remove from library
	m_Engine.getPluginLibraryServer().fromGuiSetFileIsInLibrary( fileNameAndPath, false );
	// remove from shared files
	m_Engine.getPluginFileShareServer().fromGuiSetFileIsShared( fileNameAndPath, false );
	// remove from transfers
	m_Engine.getPluginFileShareServer().fileAboutToBeDeleted( fileNameAndPath );
	// remove from assets
	removeAsset( fileNameAndPath, !shredFile );
	if( shredFile )
	{
		m_FileShredder.shredFile( fileNameAndPath );
	}
}

//============================================================================
void AssetBaseMgr::fromGuiFileHashGenerated( std::string& fileNameAndPath, int64_t fileLen, VxSha1Hash& fileHash )
{
	lockResources();
	for( auto* assetInfo : m_AssetBaseInfoList )
	{
        if( fileLen == assetInfo->getAssetLength() && fileNameAndPath == assetInfo->getAssetNameAndPath() )
		{
			assetInfo->setAssetHashId( fileHash );
			updateDatabase( assetInfo );
			break;
		}
	}

	unlockResources();
}

//============================================================================
void AssetBaseMgr::requestFileHash( AssetBaseInfo* assetInfo )
{
	GetSha1GeneratorMgr().generateSha1( assetInfo->getAssetUniqueId(), assetInfo->getAssetName(), assetInfo->getFileNameAndPath(), this);
}

//============================================================================
void AssetBaseMgr::callbackSha1GenerateResult( ESha1GenResult sha1GenResult, VxGUID& assetId, Sha1Info& sha1Info )
{
	if( eSha1GenResultNoError == sha1GenResult )
	{
		lockResources();

		// move from waiting to completed
		bool wasMoved{ false };
		for( auto iter = m_WaitingForHastList.begin(); iter != m_WaitingForHastList.end(); ++iter )
		{
			AssetBaseInfo* inListAssetBaseInfo = *iter;
			if( assetId == inListAssetBaseInfo->getAssetUniqueId() && sha1Info.getFileNameAndPath() == inListAssetBaseInfo->getAssetNameAndPath() )
			{
				AssetBaseInfo* toMoveAssetInfo = inListAssetBaseInfo;
				m_WaitingForHastList.erase( iter );
				toMoveAssetInfo->setAssetHashId( sha1Info.getSha1Hash() );
				m_AssetBaseInfoList.emplace_back( toMoveAssetInfo );
				updateDatabase( toMoveAssetInfo );
				announceAssetAdded( toMoveAssetInfo, true );
				wasMoved = true;
				break;
			}
		}

		bool wasFound{ false };
		if( !wasMoved )
		{
			for( auto* assetInfo : m_AssetBaseInfoList )
			{
				if( assetId == assetInfo->getAssetUniqueId() && sha1Info.getFileNameAndPath() == assetInfo->getAssetNameAndPath() )
				{
					assetInfo->setAssetHashId( sha1Info.getSha1Hash() );
					updateDatabase( assetInfo );
					wasFound = true;
					break;
				}
			}
		}

		unlockResources();
		if( wasMoved || wasFound )
		{
			m_Engine.getPluginFileShareServer().fromGuiFileHashGenerated( sha1Info.getFileNameAndPath(), sha1Info.getFileLen(), sha1Info.getSha1Hash() );
		}
	}
	else
	{
		LogMsg( LOG_VERBOSE, "AssetBaseMgr::%s failed %s", __func__, DescribeSha1GenResult( sha1GenResult ) );
	}
}

//============================================================================
void AssetBaseMgr::deleteThumbAsset( VxGUID& thumbId )
{
	if( !thumbId.isValid() )
	{
		return;
	}

	if( isEmoticonThumbnail( thumbId ) )
	{
		return;
	}

	m_Engine.getThumbMgr().deleteThumb( thumbId );

	AssetBaseInfo* assetInfo{ nullptr };
	AssetBaseInfo assetBaseInfo;
	bool found{ false };
	lockResources();
	for( auto iter = m_AssetBaseInfoList.begin(); iter != m_AssetBaseInfoList.end(); ++iter )
	{
		assetInfo = *iter;
		if( assetInfo->isThumbAsset() && assetInfo->getAssetUniqueId() == thumbId )
		{
			assetBaseInfo = *assetInfo;
			found = true;
			break;
		}
	}

	unlockResources();
}
