#pragma once
//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <AssetBase/BaseXferInterface.h>

class PluginBase;

class ThumbXferInterface : public BaseXferInterface
{
public:
    ThumbXferInterface( PluginBase& pluginBase );

    VxMutex&                    getAssetXferMutex( void ) override;
    EPluginType                 getPluginType( void ) override;
    EPluginType                 getAssetOverridePluginType( void ) override { return ePluginTypeInvalid; }
    std::string                 getAssetXferDbName( void ) override;
    std::string                 getAssetXferThreadName( void ) override;

    bool                        txPacket( const VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, EPluginType overridePluginType = ePluginTypeInvalid ) override;

    PluginBase&                 m_PluginBase;
};