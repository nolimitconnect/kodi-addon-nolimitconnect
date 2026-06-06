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

#include "VxConnectInfo.h"
#include "VxOnlineStatusFlags.h"	

#include <memory>
#include <memory.h>

class VxSktBase;

//! \public enumerated application State
enum EAppState
{
	eAppStateInvalid		= 0,	// unknown or disabled
	eAppStateStartup		= 1,	// app has started
	eAppStateShutdown		= 2,	// app shutdown
	eAppStateSleep			= 3,	// app sleep
	eAppStateWake			= 4,	// app wake
	eAppStatePause			= 5,	// pause app
	eAppStateResume			= 6,	// resume
	eAppStatePermissionErr	= 7,	// disabled by permission
	eAppStateInactive		= 8,	// inactive due to no files or error etc

	eMaxAppState // must be last.. maximum application states
};

#define USE_PLUGIN_WEB_SERVER		1
#define USE_PLUGIN_RELAY			1
#define USE_PLUGIN_CAM_SERVER		1
#define USE_PLUGIN_INST_MSG			1
#define USE_PLUGIN_VOICE_PHONE		1
#define USE_PLUGIN_VIDEO_PHONE		1
#define USE_PLUGIN_TRUTH_OR_DARE	1
#define USE_PLUGIN_FILE_OFFER		1
#define USE_PLUGIN_FILE_SERVER		1
#define USE_PLUGIN_STORY_BOARD		1

// permission bits
// 0000		disabled or ignore
// 0001		anonymous or anyone
// 0010		guest 
// 0011		friends
// 0100		admin

#pragma pack(push)
#pragma pack(1)
//! 24 bytes in size
//! 48 max possible plugins
#define PERMISSION_ARRAY_SIZE 24
/// class to handle plugin permissions array
class PluginPermission
{
public:
	PluginPermission();
    PluginPermission( const PluginPermission &rhs ) = default;
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );
    PluginPermission&           operator =( const PluginPermission &rhs );

	//! set type of permission user has set for given plugin
    void						setPluginPermission( enum EPluginType pluginType, enum EFriendState eFriendState );

	void						setPluginPermissions( uint8_t * permissions )	{ memcpy( m_au8Permissions, permissions, PERMISSION_ARRAY_SIZE ); }
	uint8_t *					getPluginPermissions( void )					{ return m_au8Permissions; }

	//! reset permissions to default values
	void						setPluginPermissionsToDefaultValues( void );

protected:
	//=== vars ===//
	uint8_t						m_au8Permissions[ PERMISSION_ARRAY_SIZE ];
};

//  size
// +   24 bytes PluginPermission
// +    1 byte VxOnlineStatusFlags
// +    1 byte m_JoinedFlags
// +    1 byte m_HostAdminFlags;	
// +    1 byte m_NetIdentRes1;	
// +    2 bytes m_u16AppVersion;	
// +    2 bytes m_u16PingTimeMs;	
// 
// +    8 bytes m_LastSessionTime;
// +    8 bytes m_GroupieInfoModifiedTimeMs
// 48 bytes to here

// +    4 bytes m_TruthAcceptCnt;
// +    4 bytes m_TruthRejectCnt;
// +    4 bytes m_DareAcceptCnt;	
// +    4 bytes m_DareRejectCnt;	
// 64 bytes to here

// +    8 bytes m_NetIdentRes2;
// +    8 bytes m_NetIdentRes3;
// +    8 bytes m_NetIdentRes4;	
// +    8 bytes m_NetIdentRes5;	
// =   96 bytes bytes total
// 
// 336 bytes VxConnectInfo + 96
// = 432 bytes total // 400 bytes total

/// network indentiy of contact
class VxNetIdent : public VxConnectInfo, public PluginPermission, public VxOnlineStatusFlags
{
public:
	VxNetIdent();
	VxNetIdent(const VxNetIdent &rhs );
    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

	VxNetIdent&					operator =( const VxNetIdent& rhs );
	bool                        operator ==( const VxNetIdent& rhs ) const;
	bool                        operator != ( const VxNetIdent& rhs ) const;

	VxConnectInfo&				getConnectInfo( void ) { return *this; }

	bool                        isMyself( void );

	bool						isOnline( void ) { return isDirectConnected() || isRelayed(); }
	bool						isDirectConnected( void );
	bool						isRelayed( void );

	bool						canDirectConnectToUser( void );

	void						clearIsAdminAvail( void );
	void						setIsAdminAvail( EHostType hostType, bool isAdminAvail );
	bool						getIsAdminAvail( EHostType hostType );
	uint8_t						getAdminAvailFlags( void )					{ return m_AdminAvailFlags; }

	void						clearIsJoined( void );
	void						setIsJoined( EHostType hostType, bool isJoined );
	bool						getIsJoined( EHostType hostType );
	bool						isJoinedAny( void );

	void						setTruthAcceptCount( uint32_t truthCnt )	{ m_TruthAcceptCnt = truthCnt; }
	uint32_t					getTruthAcceptCount( void )					{ return m_TruthAcceptCnt; }
	void						setTruthRejectCount( uint32_t truthCnt )	{ m_TruthRejectCnt = truthCnt; }
	uint32_t					getTruthRejectCount( void )					{ return m_TruthRejectCnt; }

	void						setDareAcceptCount( uint32_t dareCnt )		{ m_DareAcceptCnt = dareCnt; }
	uint32_t					getDareAcceptCount( void )					{ return m_DareAcceptCnt; }
	void						setDareRejectCount( uint32_t dareCnt )		{ m_DareRejectCnt = dareCnt; }
	uint32_t					getDareRejectCount( void )					{ return m_DareRejectCnt; }

	bool						isVxNetIdentMatch( const VxNetIdent& oOtherIdent ) const;

	bool						isPluginEnabled( enum EPluginType ePlugin );
	//! get type of permission user has set for given plugin
	EFriendState				getPluginPermission( enum EPluginType pluginType );

    EPluginAccess			    getHisAccessPermissionFromMe( enum EPluginType pluginType );
    bool						isHisAccessAllowedFromMe( enum EPluginType pluginType );

    EPluginAccess			    getMyAccessPermissionFromHim( enum EPluginType pluginType );
    bool						isMyAccessAllowedFromHim( enum EPluginType pluginType );


	//! if was anonymouse upgrade to guest friendship
	void                        upgradeToGuestFriendship( void );

	bool						isIgnored();
	bool						isAnonymous();
	bool						isGuest();
	bool						isFriend();
	bool						isAdministrator();
	//! set my permissions to him as ignored
	void						makeIgnored();
	//! set my permissions to him as Anonymous
	void						makeAnonymous();
	//! set my permissions to him as Guest
	void						makeGuest();
	//! set my permissions to him as Friend
	void						makeFriend();
	//! set my permissions to him as Administrator
	void						makeAdministrator();
	//! wants to be friend
	bool						wantsToBeFriend();
	//! wants to be Administrator
	bool						wantsToBeAdministrator();


	//! set permission level I have given to friend
	void						setMyFriendshipToHim( enum EFriendState eFriendState );
	//! get permission level I have given to friend
	EFriendState				getMyFriendshipToHim( void );

	//! set permission level he has given to me
	void						setHisFriendshipToMe( EFriendState eFriendState );

	//! get permission level he has given to me
	EFriendState				getHisFriendshipToMe( void );

	//! reverse the permissions
	void						reversePermissions( void );
	//! return string with friend state He has given Me
	void						describeHisFriendshipToMe( std::string& strRetPermission );
	//! return string with friend state He has given Me
	const char*					describeHisFriendshipToMe( void );
	//! return string with friend state I have given Him
	void						describeMyFriendshipToHim( std::string& strRetPermission );
	//! return string with friend state I have given Him
	const char*					describeMyFriendshipToHim( void );

	void						setPingTimeMs( uint16_t pingTime );
	uint16_t					getPingTimeMs( void );

	void						setLastSessionTimeMs( int64_t lastSessionTimeGmtMs )			{ m_LastSessionTimeGmtMs = lastSessionTimeGmtMs; }
	int64_t					    getLastSessionTimeMs( void )									{ return m_LastSessionTimeGmtMs; }

	void						setLastGroupieInfoModifiedTimeMs( int64_t lastModifiedTimeMs )  { if( lastModifiedTimeMs > m_GroupieInfoModifiedTimeMs ) { m_GroupieInfoModifiedTimeMs = lastModifiedTimeMs; } }
	int64_t					    getLastGroupieInfoModifiedTimeMs( void )						{ return m_GroupieInfoModifiedTimeMs; }

	void						debugDumpIdent( void );

	bool                        isValidNetIdent( void );
	bool						isOnlineNameValid( void );

    bool						canRequestJoin( enum EHostType hostType );
    bool						canJoinImmediate( enum EHostType hostType ); // request to join will be granted immediate because have sufficient permission

	EPluginAccess			    getPluginAccessState( enum EPluginType pluginType, enum EFriendState eFriendState );

	std::string					describeUser( void );

	bool						userIsHosting( enum EHostType hostType );

	bool						requiresAnOpenPort( void );

    void 						dumpPermissions( bool justHosts = false );

private:

	//=== vars ===//
	uint8_t						m_AdminAvailFlags{ 0 };
	uint8_t						m_JoinedFlags{ 0 };
	uint8_t						m_NetIdentRes1{ 0 };

    uint16_t					m_u16AppVersion{ 0 };
	uint16_t					m_u16PingTimeMs{ 0 };	

	int64_t					    m_LastSessionTimeGmtMs{ 0 };
	int64_t                     m_GroupieInfoModifiedTimeMs{ 0 };

	uint32_t					m_TruthAcceptCnt{ 0 };
	uint32_t					m_TruthRejectCnt{ 0 };
	uint32_t					m_DareAcceptCnt{ 0 };
	uint32_t					m_DareRejectCnt{ 0 };
    
	// for future use
	int64_t					    m_NetIdentRes2{ 0 };
	int64_t						m_NetIdentRes3{ 0 };
	int64_t					    m_NetIdentRes4{ 0 };
	int64_t						m_NetIdentRes5{ 0 };
};

#pragma pack(pop)

class IHackReportCallbackInterface
{
public:
	virtual void                reportHackOffense( EHackerLevel hackerLevel, EHackerReason hackerReason, std::string ipAddr, std::string hackDescription ) = 0;
};

void VxSetHackReportCallback( IHackReportCallbackInterface* hackReportCallback );

//! report a attempt to hack or add spyware etc
///						EHackerLevel hackerLevel		// 1=severe 2=medium 3=suspicious
///						HackerReason hackerReason		// reason hack reported
///						ipAddr							// ip address of hacker
///						hackDescription					// description of hack attempt
int32_t VxReportHack(	enum EHackerLevel hackerLevel, enum EHackerReason hackerReason, std::shared_ptr<VxSktBase>& sktBase, const char* hackDescription, ... );
int32_t VxReportHack(	enum EHackerLevel hackerLevel, enum EHackerReason hackerReason, SOCKET skt, const char* ipAddr, const char* hackDescription, ... );



