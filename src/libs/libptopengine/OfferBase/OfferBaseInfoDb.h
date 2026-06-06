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

#include <config_appcorelibs.h>

#include <CoreLib/DbBase.h>
#include <CoreLib/VxGUID.h>
#include <GuiInterface/IDefs.h>

class OfferBaseInfo;
class OfferBaseMgr;
class VxGUID;
class VxSha1Hash;

class OfferBaseInfoDb : public DbBase
{
public:
	OfferBaseInfoDb( OfferBaseMgr& mgr );
	virtual ~OfferBaseInfoDb() = default;

	void						lockOfferInfoDb( void )					{ m_OfferBaseInfoDbMutex.lock(); }
	void						unlockOfferInfoDb( void )				{ m_OfferBaseInfoDbMutex.unlock(); }

	void						addOffer(	VxGUID&			offerId,
											VxGUID&			creatorId, 
											VxGUID&			historyId, 
                                            VxGUID&			thumbId, 
											VxGUID&			sendToId, 
											VxGUID&			assetId, 
                                            const char*     assetName,
										    const char*     fileNameAndPath,
											int64_t			assetLen, 
											uint32_t		assetType, 							
											VxSha1Hash&		hashId, 
											uint32_t		locationFlags, 
                                            uint32_t		attributedFlags, 
                                            int8_t          pluginType,
                                            const char*     offerMsg,
                                            int64_t			offerExpires,
											EOfferResponse  offerResponse,
											EOfferMgrType	offerMgr,
                                            int             isTemp,
                                            int64_t			createdTimestamp = 0,
                                            int64_t			modifiedTimestamp = 0,     
                                            int64_t			accessedTimestamp = 0,          
                                            const char*     assetTag = "",
											EOfferSendState sendState = eOfferSendStateNone);

	void 						addOffer( OfferBaseInfo* offerInfo );

	void						removeOffer( const char* assetName );
	void						removeOffer( VxGUID& offerId );
	void						removeOffer( OfferBaseInfo* offerInfo );

	void						getAllOffers( std::vector<OfferBaseInfo*>& OfferBaseOfferList );
	void						purgeAllOffers( void ); 
	void						updateOfferSendState( VxGUID& assetId, EOfferSendState sendState );

protected:
	virtual int32_t				onCreateTables( int iDbVersion );
	virtual int32_t				onDeleteTables( int iOldVersion );
	void						insertOfferInTimeOrder( OfferBaseInfo* offerInfo, std::vector<OfferBaseInfo*>& assetList );

	OfferBaseMgr&				m_OfferMgr;
	VxMutex						m_OfferBaseInfoDbMutex;
};

