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

// uncomment to show ident list mgr lock/unlock
//#define DEBUG_IDENT_LIST_MGR_LOCK 1

#include <GuiInterface/IDefs.h>
#include <CoreLib/VxMutex.h>

class VxGUID;
class P2PEngine;
class IdentListMgrBase
{
public:
    IdentListMgrBase( P2PEngine& engine );
    virtual ~IdentListMgrBase() = default;

    void                        lockIdentList( void ) { m_IdentListMutex.lock(); }
    void                        unlockIdentList( void ) { m_IdentListMutex.unlock(); }

    virtual void                updateIdent( VxGUID& onlineId, int64_t timestamp ) {};
    virtual void                removeIdent( VxGUID& onlineId ) {};
    virtual void                removeAll( void ) {};

    virtual void                setIdentListType( enum EUserViewType listType ) { m_ListType = listType; }

protected:
    virtual void                onUpdateIdent( VxGUID& onlineId, int64_t timestamp );
    virtual void                onRemoveIdent( VxGUID& onlineId );

    P2PEngine&                  m_Engine;
    VxMutex                     m_IdentListMutex;
    EUserViewType               m_ListType{ eUserViewTypeNone };
};

