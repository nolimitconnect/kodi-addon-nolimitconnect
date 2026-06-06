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

#include <BaseInfo/BaseJoinInfo.h>
#include <GuiInterface/IDefs.h>
#include <CoreLib/GroupieId.h>

#define HOST_FLAG_DEFAULT_HOST			0x0001
#define HOST_FLAG_IS_TEMP			    0x0002

class HostJoinInfo : public BaseJoinInfo
{
public:
	HostJoinInfo();
	HostJoinInfo( const HostJoinInfo& rhs );

	HostJoinInfo&				operator=( const HostJoinInfo& rhs ); 

    bool                        isValid( void )                                     { return m_NetIdent != nullptr;  }
    bool                        isUrlValid( void );

    virtual void			    setNetIdent( VxNetIdent* netIdent )                 { m_NetIdent = netIdent; }
    virtual VxNetIdent*         getNetIdent( void )                                 { return m_NetIdent; }

    virtual void			    setHostFlags( uint32_t hostFlags )                  { m_HostFlags = hostFlags; }
    virtual uint32_t			getHostFlags( void )                                { return m_HostFlags; }

    void						setIsDefaultHost( bool isDefault )	                { if( isDefault ) m_HostFlags |= HOST_FLAG_DEFAULT_HOST; else m_HostFlags &= ~HOST_FLAG_DEFAULT_HOST; }
    bool						isDefaultHost( void )				                { return m_HostFlags & HOST_FLAG_DEFAULT_HOST ? true : false; }
    void						setIsTemp( bool isTemp )	                        { if( isTemp ) m_HostFlags |= HOST_FLAG_IS_TEMP; else m_HostFlags &= ~HOST_FLAG_IS_TEMP; }
    bool						isTemp( void )				                        { return m_HostFlags & HOST_FLAG_IS_TEMP ? true : false; }

    void                        setGroupieId( GroupieId& groupieId )                { m_GroupieId = groupieId; }
    GroupieId&                  getGroupieId( void )                                { return m_GroupieId; }

    virtual void			    setUserUrl( std::string userUrl )                   { if( userUrl.empty() ) return; m_UserUrl = userUrl; }
    virtual std::string&	    getUserUrl( void )                                  { return m_UserUrl; }
    virtual void			    setFriendState( enum EFriendState friendshipToHim ) { m_FriendState = friendshipToHim; }
    virtual EFriendState	    getFriendState( void )                              { return m_FriendState; }

    // temporaries
    virtual void				setConnectionId( VxGUID& connectionId )             { m_ConnectionId = connectionId; }
    virtual VxGUID&				getConnectionId( void )                             { return m_ConnectionId; }
    virtual void				setSessionId( VxGUID& sessionId )                   { m_SessionId = sessionId; }
    virtual VxGUID&				getSessionId( void )                                { return m_SessionId; }

    virtual std::string         describeHostJoin( void );

protected:
	//=== vars ===//
    VxNetIdent*                 m_NetIdent{ nullptr };
    EFriendState                m_FriendState{ eFriendStateIgnore };
    uint32_t                    m_HostFlags{ 0 };
    GroupieId                   m_GroupieId;
    std::string                 m_UserUrl;

    // temporaries
    VxGUID                      m_ConnectionId;
    VxGUID                      m_SessionId;
};
