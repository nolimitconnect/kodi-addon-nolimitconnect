//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <config_appcorelibs.h>
#include "OfferBaseXferDb.h"
#include "OfferBaseInfo.h"

#include <CoreLib/VxDebug.h>

namespace
{
	std::string 		TABLE_OFFER_XFER	 			= "offer_xfer";
	std::string 		CREATE_COLUMNS_OFFER_XFER		= " (offerId TEXT PRIMARY KEY ) ";
}

//============================================================================
OfferBaseXferDb::OfferBaseXferDb( const char* stateDbName )
: DbBase( stateDbName )
{
}

//============================================================================
//! create tables in database 
int32_t OfferBaseXferDb::onCreateTables( int iDbVersion )
{
	lockOfferBaseXferDb();
    std::string strCmd = "CREATE TABLE " + TABLE_OFFER_XFER + CREATE_COLUMNS_OFFER_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockOfferBaseXferDb();
	return rc;
}

//============================================================================
// delete tables in database
int32_t OfferBaseXferDb::onDeleteTables( int iOldVersion ) 
{
	lockOfferBaseXferDb();
    std::string strCmd = "DROP TABLE IF EXISTS " + TABLE_OFFER_XFER;
    int32_t rc = sqlExec(strCmd);
	unlockOfferBaseXferDb();
	return rc;
}

//============================================================================
void OfferBaseXferDb::purgeAllOfferBaseXfer( void ) 
{
	lockOfferBaseXferDb();
    std::string strCmd = "DELETE FROM " + TABLE_OFFER_XFER;
    int32_t rc = sqlExec( strCmd );
	unlockOfferBaseXferDb();
	if( rc )
	{
		LogMsg( LOG_ERROR, "OfferBaseXferDb::purgeAllOfferBaseXfer error %d", rc );
	}
	else
	{
		LogMsg( LOG_INFO, "OfferBaseXferDb::purgeAllOfferBaseXfer success" );
	}
}

//============================================================================
void OfferBaseXferDb::removeOffer( VxGUID& assetOfferId )
{
	std::string strId;
	assetOfferId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM offer_xfer WHERE offerId=?", bindList );
}

//============================================================================
void OfferBaseXferDb::addOffer( VxGUID& assetOfferId )
{
	std::string strId;
	assetOfferId.toHexString( strId );
	DbBindList bindList( strId.c_str() );
	sqlExec( "DELETE FROM offer_xfer WHERE offerId=?", bindList );

	int32_t rc  = sqlExec( "INSERT INTO offer_xfer (offerId) values(?)",
		bindList );
	if( rc )
	{
		LogMsg( LOG_ERROR, "OfferBaseXferDb::addOffer error %d", rc );
	}
}

//============================================================================
void OfferBaseXferDb::getAllOffers( std::vector<VxGUID>& assetList )
{
	assetList.clear();
	std::string strId;
	VxGUID uniqueId;
	lockOfferBaseXferDb();
	DbCursor * cursor = startQuery( "SELECT * FROM offer_xfer" );
	if( NULL != cursor )
	{
		while( cursor->getNextRow() )
		{
			strId = cursor->getString( 0 );
			uniqueId.fromVxGUIDHexString( strId.c_str() );
			assetList.push_back( uniqueId );
		}

		cursor->close();
	}

	unlockOfferBaseXferDb();
}


