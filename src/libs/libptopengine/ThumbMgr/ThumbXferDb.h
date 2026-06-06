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

class ThumbInfo;

class ThumbXferDb : public DbBase
{
public:
	ThumbXferDb();
	virtual ~ThumbXferDb();

	void						lockThumbXferDb( void )			        { m_ThumbXferDbMutex.lock(); }
	void						unlockThumbXferDb( void )			    { m_ThumbXferDbMutex.unlock(); }

	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

	void 						addThumb( VxGUID& assetUniqueId );
	void						removeThumb( VxGUID& assetUniqueId );

	void						getAllThumbs( std::vector<VxGUID>& assetList );
	void						purgeAllThumbXfer( void ); 

protected:
	VxMutex						m_ThumbXferDbMutex;
};

