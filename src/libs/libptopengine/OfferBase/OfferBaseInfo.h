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

#include <AssetMgr/AssetInfo.h>

class FileInfo;

class OfferBaseInfo : public AssetInfo
{
public:
    OfferBaseInfo() = default;
    OfferBaseInfo( const OfferBaseInfo& rhs );
    OfferBaseInfo( std::string fileName, std::string fileNameAndPath );
    OfferBaseInfo( std::string fileName, std::string fileNameAndPath, uint64_t assetLen, uint16_t assetType );
    OfferBaseInfo( FileInfo& fileInfo );

    OfferBaseInfo& operator=( const OfferBaseInfo& rhs );

    virtual bool				isValid( bool logErrIfInvalid = true ) override;
    virtual bool				isSessionMatch( OfferBaseInfo& rhs );

    virtual bool                addToBlob( PktBlobEntry& blob ) override;
    virtual bool                extractFromBlob( PktBlobEntry& blob ) override;

    void						setIsRemoteInitiated( bool bIsRemoteInitiated ) { m_OfferMgr = bIsRemoteInitiated ? eOfferMgrClient : eOfferMgrHost; }
	bool						getIsRemoteInitiated( void )			{ return m_OfferMgr == eOfferMgrClient; }

	virtual void				setOfferType( EOfferType assetType )	{ setAssetType( (EAssetType)assetType ); }
	virtual EOfferType			getOfferType( void )					{ return (EOfferType)getAssetType(); }

    virtual void				setOfferName( std::string& assetName )  { setAssetName( assetName ); }
    virtual void				setOfferFile( std::string& fullFileName )  { setAssetNameFromFileNameAndPath( fullFileName ); }
    virtual std::string&        getOfferName( void )                    { return getAssetName(); }
    virtual std::string&        getOfferFile( void )                    { return getAssetName(); }

    virtual void				setOfferLength( int64_t assetLength )   { setAssetLength( assetLength ); }
    virtual int64_t				getOfferLength( void )                  { return getAssetLength(); }

    virtual void				setOfferHashId( VxSha1Hash& hashId )    { setAssetHashId( hashId ); }
    virtual void				setOfferHashId( uint8_t* hashId )       { setAssetHashId( hashId ); }
    virtual VxSha1Hash&         getOfferHashId( void )                  { return getAssetHashId(); }

    virtual void				setOfferTag( std::string assetTag )     { setAssetTag( assetTag.c_str() ); }
    virtual std::string&        getOfferTag( void )                     { return getAssetTag(); }
    virtual bool                hasOfferTag( void )                     { return !getAssetTag().empty(); }

    virtual void				setOfferSendState( EOfferSendState sendState ) { setAssetSendState( (EAssetSendState)sendState ); }
    virtual EOfferSendState		getOfferSendState( void )               { return (EOfferSendState)getAssetSendState(); }

    virtual std::string         getRemoteOfferName( void )              { return getRemoteAssetName(); }
 
    virtual bool                isPhoneTypePlugin( void ); // behaves like phone with ringing

    virtual bool                isFileOffer( void )                     { return isFileAsset(); }

    bool                        isAccepted( void );
    bool                        isRejected( void );
    bool                        isExpired( void );
    bool                        isWaitingForResponse( void );

    virtual void				setIsSharedFileOffer( bool isSharedOffer ) { setIsSharedFileAsset( isSharedOffer ); }
    virtual bool				isSharedFileOffer( void )               { return isSharedFileAsset(); }

    virtual void				setOfferMsg( std::string offerMsg )     { m_OfferMsg = offerMsg; }
    virtual std::string&		getOfferMsg( void )                     { return m_OfferMsg; }
    virtual bool                hasOfferMsg( void )                     { return !m_OfferMsg.empty(); }

    virtual void				setOfferExpireTime( int64_t expireTime ) { m_OfferExpireTime = expireTime; }
    virtual int64_t             getOfferExpireTime( void )              { return m_OfferExpireTime; }

    virtual void				setOfferId( VxGUID& sessionId )         { m_OfferId = sessionId; }
    virtual void				setOfferId( const char* sessionId )     { m_OfferId.fromVxGUIDHexString( sessionId ); }
    virtual VxGUID&             getOfferId( void )                      { return m_OfferId; }

    virtual void                setOfferResponse( EOfferResponse offerResponse ) { m_OfferResponse = offerResponse; }
    virtual EOfferResponse      getOfferResponse( void )                { return m_OfferResponse; }

    virtual void                setOfferMgr( EOfferMgrType offerMgr )   { m_OfferMgr = offerMgr; }
    virtual EOfferMgrType       getOfferMgr( void )                     { return m_OfferMgr; }

    virtual void                fillOfferSend( EPluginType pluginType, VxNetIdent& netIdent );

    void						setOfferTimestamp( int64_t timems )     { m_OfferTimestamp = timems; }
	int64_t						getOfferTimestamp( void )               { return m_OfferTimestamp; }

    void						setOfferResponseTimestamp( int64_t timems ) { m_OfferResponseTimestamp = timems; }
    int64_t						getOfferResponseTimestamp( void )       { return m_OfferResponseTimestamp; }

    VxGUID&                     getToOnlineId( void )                   { return m_OfferMgr == eOfferMgrHost ? getCreatorId() : getHistoryId(); }
    VxGUID&                     getFromOnlineId( void )                 { return m_OfferMgr == eOfferMgrHost ? getHistoryId() : getCreatorId(); }

protected:
    EOfferMgrType               m_OfferMgr{ eOfferMgrNotSet };
    VxGUID                      m_OfferId;
    int64_t                     m_OfferExpireTime{ 0 };
    std::string                 m_OfferMsg;
    EOfferResponse              m_OfferResponse{ eOfferResponseNotSet };
    int64_t                     m_OfferTimestamp{ 0 };
    
    int64_t                     m_OfferResponseTimestamp{ 0 }; // not sent over network
};
