//============================================================================
// Copyright (C) 2024 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "ThumbXferInterface.h"

#include <Plugins/PluginBase.h>

//============================================================================
ThumbXferInterface::ThumbXferInterface( PluginBase& pluginBase )
    : m_PluginBase( pluginBase )
{
}

//============================================================================
VxMutex& ThumbXferInterface::getAssetXferMutex( void )
{
    return m_PluginBase.getAssetXferMutex();
}

//============================================================================
EPluginType ThumbXferInterface::ThumbXferInterface::getPluginType( void )
{
    return m_PluginBase.getPluginType();
}

//============================================================================
std::string ThumbXferInterface::getAssetXferDbName( void )
{
    std::string dbName = GetPluginName( getPluginType() ); 
    dbName += "ThumDb.db3"; 
    return dbName;
}

//============================================================================
std::string ThumbXferInterface::getAssetXferThreadName( void )
{
    std::string thrdName = GetPluginName( getPluginType() ); 
    thrdName += "ThumbThrd"; 
    return thrdName;
}

//============================================================================
bool ThumbXferInterface::txPacket( const VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, EPluginType overridePluginType )
{
    return m_PluginBase.txPacket( sendToId, sktBase, pktHdr, overridePluginType );
}
