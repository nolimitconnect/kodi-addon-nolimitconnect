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

#include <GuiInterface/IDefs.h>
#include <CoreLib/VxMutex.h>

class VxGUID;
class P2PEngine;
class SktListMgrBase
{
public:
    SktListMgrBase( P2PEngine& engine );
    virtual ~SktListMgrBase() = default;

    void                        lockList( void ) { m_ListMutex.lock(); }
    void                        unlockList( void ) { m_ListMutex.unlock(); }

    virtual void                updateIdent( VxGUID& onlineId, int64_t timestamp ) {};
    virtual void                removeIdent( VxGUID& onlineId ) {};
    virtual void                removeAll( void ) {};

    virtual void                setSktListType( EUserViewType listType ) { m_ListType = listType; }

protected:
    virtual void                onUpdateIdent( VxGUID& onlineId, int64_t timestamp );
    virtual void                onRemoveIdent( VxGUID& onlineId );

    P2PEngine&                  m_Engine;
    VxMutex                     m_ListMutex;
    EUserViewType               m_ListType{ eUserViewTypeNone };
};

