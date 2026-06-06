//============================================================================
// Copyright (C) 2021 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "OfferBaseInfo.h"

#include <P2PEngine/P2PEngine.h>

#include <CoreLib/PktBlobEntry.h>
#include <CoreLib/VxTime.h>
#include <PktLib/VxCommon.h>

//============================================================================
OfferBaseInfo::OfferBaseInfo( const OfferBaseInfo& rhs )
	: AssetInfo( rhs )
	, m_OfferMgr( rhs.m_OfferMgr )
	, m_OfferId( rhs.m_OfferId )
	, m_OfferExpireTime( rhs.m_OfferExpireTime )
	, m_OfferMsg( rhs.m_OfferMsg )
	, m_OfferResponse( rhs.m_OfferResponse )
	, m_OfferTimestamp( rhs.m_OfferTimestamp )
	, m_OfferResponseTimestamp( rhs.m_OfferResponseTimestamp )
{
}

//============================================================================
OfferBaseInfo::OfferBaseInfo( std::string fileName, std::string fileNameAndPath )
	: AssetInfo( VxFileNameToAssetType( fileName ), fileName, fileNameAndPath )
{
}

//============================================================================
OfferBaseInfo::OfferBaseInfo( std::string fileName, std::string fileNameAndPath, uint64_t assetLen, uint16_t assetType )
	: AssetInfo( VxFileNameToAssetType( fileName ), fileName, fileNameAndPath, assetLen )
{
}

//============================================================================
OfferBaseInfo::OfferBaseInfo( FileInfo& fileInfo )
	: AssetInfo( fileInfo )
{
}

//============================================================================
OfferBaseInfo& OfferBaseInfo::operator=( const OfferBaseInfo& rhs )
{
	if( this != &rhs )
	{
		*((AssetInfo*)this) = rhs;
		m_OfferMgr = rhs.m_OfferMgr;
		m_OfferId = rhs.m_OfferId;
		m_OfferExpireTime = rhs.m_OfferExpireTime;
		m_OfferMsg = rhs.m_OfferMsg;
		m_OfferResponse = rhs.m_OfferResponse;
		m_OfferTimestamp = rhs.m_OfferTimestamp;
		m_OfferResponseTimestamp = rhs.m_OfferResponseTimestamp;
	}

	return *this;
}

//============================================================================
bool OfferBaseInfo::addToBlob( PktBlobEntry& blob )
{
	blob.resetWrite();
	if( AssetInfo::addToBlob( blob ) )
	{
        bool result = blob.setValue( m_OfferMgr );
		result &= blob.setValue( m_OfferId );
		result &= blob.setValue( m_OfferExpireTime );
		result &= blob.setValue( m_OfferMsg );
		result &= blob.setValue( m_OfferResponse );
		result &= blob.setValue( m_OfferTimestamp );

		return result;
	}

	return false;
}

//============================================================================
bool OfferBaseInfo::extractFromBlob( PktBlobEntry& blob )
{
	blob.resetRead();
	if( AssetInfo::extractFromBlob( blob ) )
	{
        bool result = blob.getValue( m_OfferMgr );
		result &= blob.getValue( m_OfferId );
		result &= blob.getValue( m_OfferExpireTime );
		result &= blob.getValue( m_OfferMsg );
		result &= blob.getValue( m_OfferResponse );
		result &= blob.getValue( m_OfferTimestamp );

		return result;
	}

	return false;
}

//============================================================================
void OfferBaseInfo::fillOfferSend( EPluginType pluginType, VxNetIdent& netIdent )
{
	m_PluginType = pluginType;
	m_OfferResponse = eOfferResponseNotSet;
	m_OfferMgr = eOfferMgrHost;
	setOnlineId( netIdent.getMyOnlineId() );
	setCreatorId( GetPtoPEngine().getMyOnlineId() );
	setHistoryId( netIdent.getMyOnlineId() );
	m_UniqueId.assureIsValidGUID(); // may not neccessarily be session unique if is file or thumb asset
	// offer id always needs to be unique
	m_OfferId.initializeWithNewVxGUID();
	m_OfferTimestamp = GetTimeStampMs();
}

//============================================================================
bool OfferBaseInfo::isValid( bool logErrIfInvalid )
{
	return ePluginTypeInvalid != m_PluginType && eOfferMgrNotSet != m_OfferMgr && m_UniqueId.isValid() && m_OfferId.isValid() && getOnlineId().isValid();
}

//============================================================================
bool OfferBaseInfo::isSessionMatch( OfferBaseInfo& rhs )
{
	return getOfferId() == rhs.getOfferId() && m_PluginType == rhs.getPluginType() && getOnlineId() == rhs.getOnlineId() && getOfferMgr() == rhs.getOfferMgr();
}

//============================================================================
bool OfferBaseInfo::isPhoneTypePlugin( void )          
{ 
	return m_PluginType == ePluginTypeVoicePhone ||
			m_PluginType == ePluginTypeVideoChat ||
			m_PluginType == ePluginTypeTruthOrDare; 
}

//============================================================================
bool OfferBaseInfo::isExpired( void )
{
	return m_OfferExpireTime && GetGmtTimeMs() < m_OfferExpireTime;
}

//============================================================================
bool OfferBaseInfo::isAccepted( void )
{
	return eOfferResponseAccept == m_OfferResponse;
}

//============================================================================
bool OfferBaseInfo::isRejected( void )
{
	return eOfferResponseReject == m_OfferResponse;
}

//============================================================================
bool OfferBaseInfo::isWaitingForResponse( void )
{
	return !isExpired() && eOfferResponseNotSet == m_OfferResponse;
}
