#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "VxConnectId.h"
#include "VxRelayFlags.h"
#include "VxFriendMatch.h"
#include "VxSearchFlags.h"

#define OS_APPLE_FLAG				0x01	
#define OS_ANDROID_FLAG				0x02	
#define OS_LINUX_FLAG				0x04	
#define OS_WINDOWS_FLAG				0x08	

#define MAX_ONLINE_NAME_LEN		    32	//maximum length of online name including 0 terminator
#define MAX_ONLINE_DESC_LEN		    32	//maximum length of online description including 0 terminator
#define MAX_NET_HOST_URL_LEN		64	//maximum length of a ptop url including 0 terminator

#pragma pack(push)
#pragma pack(1)

class PktBlobEntry;

class P2PEngineVersion
{
public:
    P2PEngineVersion();
    P2PEngineVersion( const P2PEngineVersion& rhs );
    P2PEngineVersion&           operator =( const P2PEngineVersion& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    uint8_t						getP2PEngineVersion( void );
    void						getP2PEngineVersion( std::string& strRetP2PEngineVersion );

private:
    uint8_t						m_u8P2PEngineVersion{ 0 };
};

class MyOSVersion
{
public:
    MyOSVersion();
    MyOSVersion( const MyOSVersion& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    MyOSVersion&                operator =( const MyOSVersion& rhs );

    uint8_t						getOSVersion( void );
    void						getOSVersion( std::string& strRetOSVersion );

private:
    uint8_t						m_u8OSVersion{0};
};

// size
// +  1 byte P2PEngineVersion
// +  1 byte MyOSVersion
// +  1 byte VxRelayFlags
// +  1 byte FriendMatch
// +  2 bytes VxSearchFlags
// + 40 bytes m_DirectConnectId
// = 46 bytes 
class VxConnectBaseInfo : public P2PEngineVersion, public MyOSVersion, public VxRelayFlags, public FriendMatch, public VxSearchFlags
{
public:
	VxConnectBaseInfo() = default;
	VxConnectBaseInfo( const VxConnectBaseInfo& rhs );
	VxConnectBaseInfo&          operator =( const VxConnectBaseInfo& rhs );

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    std::string                 getMyOnlineUrl( EHostType hostType = eHostTypeUnknown, bool appendHostSuffix = true );

	void						setMyOnlineId( VxGUID& onlineId );
	VxGUID&						getMyOnlineId();
	bool						getMyOnlineId( std::string& strRetId );
    std::string				    getMyOnlineIdHexString( void )  { return m_DirectConnectId.toHexString(); }

	bool						setOnlineIpAddress( std::string ipAddress );
    bool						setOnlineIpAddress( InetAddress& ipAddr );
    bool						getOnlineIpAddress( std::string& retIpAddress, EIpAddrType& retIpType );
    InetAddress&				getOnlineIpAddress( void );

    void                        clearOnlineIpAddress( void ); // set to empty ip address

    bool						isOnlineIpAddressValid( void )                  { return m_DirectConnectId.isIpAddressValid(); }

	uint16_t					getOnlinePort( void );
	void						getOnlinePort( std::string& strRetPort );
	void						setOnlinePort( uint16_t u16Port );

	VxConnectId&				getDirectConnectId( void )						{ return m_DirectConnectId; }

	//=== vars ===//
	VxConnectId					m_DirectConnectId;
};

// +  32 bytes Online Name
// +  32 bytes Online Mood Message
// +   8 bytes m_TimeLastContactMs
// +   2 bytes m_PrimaryLanguage
// +   1 bytes m_ContentType
// +   1 bytes m_u8Age
// +   1 bytes m_u8Gender
// +   1 bytes reserved
// +   4 bytes reserved
// +   8 bytes reserved
// =  82 bytes
// +  64 bytes (4x16 host guids)
// + 120 bytes (5x24 thumb guids and modified times)
// = 266 bytes
// +  46 bytes VxConnectBaseInfo
// = 320 bytes

class VxConnectIdent : public VxConnectBaseInfo
{
public:
	VxConnectIdent();
    VxConnectIdent( const VxConnectIdent& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    VxConnectIdent&             operator =( const VxConnectIdent& rhs );

	VxConnectBaseInfo& 			getConnectBaseInfo( void )	                        { return * ((VxConnectBaseInfo *)this); }
	VxConnectIdent& 			getConnectIdent( void )		                        { return * ((VxConnectIdent *)this); }

	void 						setOnlineName( const char* pUserName );
    char *                      getOnlineName( void )                               { return m_OnlineName; }
	void 						setOnlineDescription( const char* pUserDesc );
	char *						getOnlineDescription( void )                        { return m_OnlineDesc; }

    void						setPrimaryLanguage( ELanguageType language )        { m_PrimaryLanguage = (uint16_t)language; }
    ELanguageType			    getPrimaryLanguage( void )                          { return (ELanguageType)m_PrimaryLanguage; }

    void						setPreferredContent( EContentRating contentType )   { m_ContentType = (uint8_t)contentType; }
    EContentRating			    getPreferredContent( void )                         { return  (EContentRating)m_ContentType; }

    void						setAgeType( enum EAgeType age )                     { m_u8Age = (uint8_t)age; }
    EAgeType					getAgeType( void )                                  { return (EAgeType)m_u8Age; }

    void						setGender( EGenderType gender )                     { m_u8Gender = (uint8_t)gender; }
    EGenderType					getGender( void )                                   { return (EGenderType)m_u8Gender; }

    void						setIsRelayed( bool isRelayed )                      { m_IsRelayed = (uint8_t)isRelayed; }
    bool						isRelayed( void )                                   { return (bool)m_IsRelayed; }

	void 						setTimeLastContact( int64_t timeStamp )				{ m_TimeLastContactMs = timeStamp; }
	int64_t	    				getTimeLastContact( void )					        { return m_TimeLastContactMs; }

    /// @brief return indenty unique folder name in the form of OnlineName_GuidHexString
    std::string	    			getIdentFolderName( void );

    bool                        getThumbnailIdList( std::vector<VxGUID>& thumbIdList );
    bool                        getThumbnailPairList( std::vector<std::pair<VxGUID, int64_t>>& thumbPairList );

    bool                        hasThumbId( enum EHostType hostType );
    VxGUID&                     getThumbId( enum EHostType hostType );
    VxGUID                      getHostThumbId( enum EHostType hostType, bool defaultToAvatarThumbId );
    void                        setHostOrThumbModifiedTime( enum EHostType hostType, int64_t& timeModified );
    int64_t                     getHostOrThumbModifiedTime( enum EHostType hostType );

    bool                        hasThumbId( enum EPluginType pluginType );
    VxGUID&                     getThumbId( enum  EPluginType pluginType );
    void                        setHostOrThumbModifiedTime( enum EPluginType pluginType, int64_t& timeModified );
    int64_t                     getHostOrThumbModifiedTime( enum EPluginType pluginType );

    void                        setAvatarGuid( VxGUID& guid, int64_t timeModified )     { m_AvatarGuid = guid; setModifiedTime( m_AvatarModifiedTime, timeModified ); }
    VxGUID&                     getAvatarThumbGuid( void )                              { return m_AvatarGuid; }
    int64_t                     getAvatarThumbModifiedTime( void )                      { return m_AvatarModifiedTime; }
    bool                        isAvatarValid( void )                                   { return m_AvatarModifiedTime && m_AvatarGuid.isValid(); }
    void                        setPeerHostModifiedTime( int64_t timeModified )         { setModifiedTime( m_AvatarModifiedTime, timeModified ); }

    // if hosts a network
    void                        setNetHostGuid( VxGUID& guid )                          { m_NetHostGuid = guid; }
    VxGUID&                     getNetHostGuid( void )                                  { return m_NetHostGuid; }
    void                        setNetHostThumb( VxGUID& guid, int64_t timeModified )   { m_NetHostThumbGuid = guid; setModifiedTime( m_NetHostThumbModifiedTime, timeModified ); }
    VxGUID&                     getNetHostThumbGuid( void )                             { return m_NetHostThumbGuid; }
    int64_t                     getNetHostThumbModifiedTime( void )                     { return m_NetHostThumbModifiedTime; }
    bool                        isNetHostThumbValid( void )                             { return m_NetHostThumbModifiedTime && m_NetHostThumbGuid.isValid(); }
    void                        setNetHostModifiedTime( int64_t timeModified )          { setModifiedTime( m_NetHostThumbModifiedTime, timeModified ); }

    // if hosts a chat room
    void                        setChatRoomHostGuid( VxGUID& guid )                     { m_ChatRoomHostGuid = guid; }
    VxGUID&                     getChatRoomHostGuid( void )                             { return m_ChatRoomHostGuid; }
    void                        setChatRoomThumb( VxGUID& guid, int64_t timeModified )  { m_ChatRoomThumbGuid = guid; setModifiedTime( m_ChatRoomThumbModifiedTime, timeModified ); }
    VxGUID&                     getChatRoomThumbGuid( void )                            { return m_ChatRoomThumbGuid; }
    int64_t                     getChatRoomThumbModifiedTime( void )                    { return m_ChatRoomThumbModifiedTime; }
    bool                        isChatRoomThumbValid( void )                            { return m_ChatRoomThumbModifiedTime && m_ChatRoomThumbGuid.isValid(); }
    void                        setChatRoomHostModifiedTime( int64_t timeModified )     { setModifiedTime( m_ChatRoomThumbModifiedTime, timeModified ); }

    // if hosts a group
    void                        setGroupHostGuid( VxGUID& guid )                        { m_GroupHostGuid = guid; }
    VxGUID&                     getGroupHostGuid( void )                                { return m_GroupHostGuid; }
    void                        setGroupThumb( VxGUID& guid, int64_t timeModified )     { m_GroupThumbGuid = guid; setModifiedTime( m_GroupThumbModifiedTime, timeModified ); }
    VxGUID&                     getGroupThumbGuid( void )                               { return m_GroupThumbGuid; }
    int64_t                     getGroupThumbModifiedTime( void )                       { return m_GroupThumbModifiedTime; }
    bool                        isGroupThumbValid( void )                               { return m_GroupThumbModifiedTime && m_GroupThumbGuid.isValid(); }
    void                        setGroupHostModifiedTime( int64_t timeModified )        { setModifiedTime( m_GroupThumbModifiedTime, timeModified ); }

    // if hosts random connect
    void                        setRandomConnectGuid( VxGUID& guid )                            { m_RandomConnectGuid = guid; }
    VxGUID&                     getRandomConnectGuid( void )                                    { return m_RandomConnectGuid; }
    void                        setRandomConnectThumb( VxGUID& guid, int64_t timeModified )     { m_RandomConnectThumbGuid = guid; setModifiedTime( m_RandomConnectThumbModifiedTime, timeModified ); }
    VxGUID&                     getRandomConnectThumbGuid( void )                               { return m_RandomConnectThumbGuid; }
    int64_t                     getRandomdConnectThumbModifiedTime( void )                      { return m_RandomConnectThumbModifiedTime; }
    bool                        isRandomConnectThumbValid( void )                               { return m_RandomConnectThumbModifiedTime && m_RandomConnectThumbGuid.isValid(); }
    void                        setRandomConnectModifiedTime( int64_t timeModified )            { setModifiedTime( m_RandomConnectThumbModifiedTime, timeModified ); }

protected:
    void                        setModifiedTime( int64_t& currentModifiedTime, int64_t& timeModified )
    {
        if( currentModifiedTime < timeModified ) { currentModifiedTime = timeModified; }
    }

    //=== vars ===//
	char						m_OnlineName[ MAX_ONLINE_NAME_LEN ];	// users online name
	char						m_OnlineDesc[ MAX_ONLINE_DESC_LEN ];    // users online description
    int64_t	    				m_TimeLastContactMs{ 0 };
    uint16_t					m_PrimaryLanguage{ 0 };     // primary language user speaks
    uint8_t					    m_ContentType{ 0 };         // preferred content type
    uint8_t						m_u8Age{ 0 };
    uint8_t						m_u8Gender{ 0 };
	uint8_t					    m_IsRelayed{ 0 };
    uint32_t					m_IdentRes2{ 0 };
    int64_t	    				m_IdentRes3{ 0 };

    VxGUID                      m_NetHostGuid;
    VxGUID                      m_ChatRoomHostGuid;
    VxGUID                      m_GroupHostGuid;
    VxGUID                      m_RandomConnectGuid;

    VxGUID                      m_AvatarGuid;
    int64_t                     m_AvatarModifiedTime{ 0 };

    VxGUID                      m_NetHostThumbGuid;
    int64_t                     m_NetHostThumbModifiedTime{ 0 };
    VxGUID                      m_ChatRoomThumbGuid;
    int64_t                     m_ChatRoomThumbModifiedTime{ 0 };
    VxGUID                      m_GroupThumbGuid;
    int64_t                     m_GroupThumbModifiedTime{ 0 };
    VxGUID                      m_RandomConnectThumbGuid;
    int64_t                     m_RandomConnectThumbModifiedTime{ 0 };
};

//     8 bytes m_s64TimeTcpLastContactMs
// +   8 bytes last connect attempt
// =  16 bytes total
// + 320 bytes VxConnectIdent
// = 336 bytes total
class VxConnectInfo : public VxConnectIdent
{
public:
    VxConnectInfo() = default;
	VxConnectInfo( const VxConnectInfo& rhs );
	VxConnectInfo&              operator =( const VxConnectInfo& rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	void						setTimeLastConnectAttemptMs( int64_t timeLastAttempt );
	int64_t 					getTimeLastConnectAttemptMs( void );
	bool						isTooSoonToAttemptConnectAgain( void );

	void						setTimeLastTcpContactMs( int64_t time );
	int64_t	    				getTimeLastTcpContactMs( void );

	int64_t		    			getElapsedMsAnyContact( void );	
	int64_t			    		getElapsedMsTcpLastContact( void );

	//=== vars ===//
private:
    int64_t					    m_s64TimeLastConnectAttemptMs{0};
    int64_t					    m_s64TimeTcpLastContactMs{0};	// time of last contact via tcp
};

#pragma pack(pop)
