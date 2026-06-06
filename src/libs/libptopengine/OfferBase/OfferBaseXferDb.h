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

#include <CoreLib/DbBase.h>
#include <CoreLib/VxGUID.h>

class OfferBaseInfo;

class OfferBaseXferDb : public DbBase
{
public:
	OfferBaseXferDb( const char* stateDbName );
	virtual ~OfferBaseXferDb() = default;

	void						lockOfferBaseXferDb( void )				{ m_OfferBaseXferDbMutex.lock(); }
	void						unlockOfferBaseXferDb( void )			{ m_OfferBaseXferDbMutex.unlock(); }

	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );

	void 						addOffer( VxGUID& assetOfferId );
	void						removeOffer( VxGUID& assetOfferId );

	void						getAllOffers( std::vector<VxGUID>& assetList );
	void						purgeAllOfferBaseXfer( void ); 

protected:
	VxMutex						m_OfferBaseXferDbMutex;
};

