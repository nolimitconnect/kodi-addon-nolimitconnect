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

class BlobInfo;

class BlobXferDb : public DbBase
{
public:
	BlobXferDb( const char* stateDbName );
	virtual ~BlobXferDb() = default;

	void						lockBlobXferDb( void )			    { m_BlobXferDbMutex.lock(); }
	void						unlockBlobXferDb( void )			{ m_BlobXferDbMutex.unlock(); }

	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

	void 						addBlob( VxGUID& assetUniqueId );
	void						removeBlob( VxGUID& assetUniqueId );

	void						getAllBlobs( std::vector<VxGUID>& assetList );
	void						purgeAllBlobXfer( void ); 

protected:
	VxMutex						m_BlobXferDbMutex;
};

