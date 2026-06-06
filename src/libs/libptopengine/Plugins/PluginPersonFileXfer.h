#pragma once
//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseFiles.h"

#include "FileInfoPersonFileXferMgr.h"

class PluginPersonFileXfer : public PluginBaseFiles
{
public:

    PluginPersonFileXfer( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType );
    ~PluginPersonFileXfer() override = default;

    EMediaModule			    getMediaModule( void ) override { return eMediaModuleInvalid; }

    void                        onPktPluginOfferReply( std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, VxNetIdent* netIdent ) override;

protected:
	FileInfoPersonFileXferMgr	m_FileInfoPersonFileXferMgr;
};
