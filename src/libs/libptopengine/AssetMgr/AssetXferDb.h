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

#include <config_appcorelibs.h>

#include <CoreLib/DbBase.h>
#include <CoreLib/VxGUID.h>

class AssetInfo;

class AssetXferDb : public DbBase
{
public:
	AssetXferDb();
	virtual ~AssetXferDb();

	void						lockAssetXferDb( void )			{ m_AssetXferDbMutex.lock(); }
	void						unlockAssetXferDb( void )			{ m_AssetXferDbMutex.unlock(); }

	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

	void 						addAsset( VxGUID& assetUniqueId );
	void						removeAsset( VxGUID& assetUniqueId );

	void						getAllAssets( std::vector<VxGUID>& assetList );
	void						purgeAllAssetXfer( void ); 

protected:
	VxMutex						m_AssetXferDbMutex;
};

