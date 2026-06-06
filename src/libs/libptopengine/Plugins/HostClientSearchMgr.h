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

#include "HostBaseMgr.h"

#include <CoreLib/VxGUIDList.h>
#include <CoreLib/VxMutex.h>

class ConnectionMgr;
class P2PEngine;
class PluginMgr;
class VxNetIdent;
class PluginBase;
class VxPktHdr;

class HostClientSearchMgr : public HostBaseMgr
{
public:
    HostClientSearchMgr( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, PluginBase& pluginBase );
	virtual ~HostClientSearchMgr() = default;

    virtual void                onPktHostSearchReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};
    virtual void                onPktPluginSettingReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) {};

protected:

    //=== vars ===//
    VxMutex                     m_SearchMutex;
    VxGUIDList                  m_SearchList;
};

