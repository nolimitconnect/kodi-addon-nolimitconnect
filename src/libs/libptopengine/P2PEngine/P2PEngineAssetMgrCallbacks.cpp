//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <AssetMgr/AssetInfo.h>


//============================================================================
void P2PEngine::callbackFileWasShredded( std::string& fileName )
{
}

//============================================================================
void P2PEngine::callbackSharedFileTypesChanged( uint16_t fileTypes )
{
}

//============================================================================
void P2PEngine::callbackSharedPktFileListUpdated( void )
{
}

//============================================================================
void P2PEngine::callbackAssetAdded( AssetBaseInfo* assetInfo )
{
    IToGui::getIToGui().toGuiAssetAdded( assetInfo );
}

//============================================================================
void P2PEngine::callbackAssetRemoved( AssetBaseInfo* assetInfo )
{
    IToGui::getIToGui().toGuiAssetAction( eAssetActionRemoveFromAssetMgr, assetInfo->getAssetUniqueId(), 0 );
}

//============================================================================
void P2PEngine::callbackAssetHistory( void * /*userData*/, AssetBaseInfo* assetInfo )
{
	IToGui::getIToGui().toGuiAssetSessionHistory( assetInfo );
}

