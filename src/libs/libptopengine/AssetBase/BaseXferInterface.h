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

#include <string>
#include <memory>

class VxGUID;
class VxNetIdent;
class VxPktHdr;
class VxSktBase;

class BaseXferInterface
{
public:
    virtual VxMutex&            getAssetXferMutex( void ) = 0;
    virtual EPluginType         getPluginType( void ) = 0;
    virtual EPluginType         getAssetOverridePluginType( void ) { return ePluginTypeInvalid; }
    virtual std::string         getAssetXferDbName( void ) { std::string dbName = GetPluginName( getPluginType() ); dbName += "Db.db3"; return dbName; }
    virtual std::string         getAssetXferThreadName( void ) { std::string thrdName = GetPluginName( getPluginType() ); thrdName += "Thrd"; return thrdName; }

    virtual bool                txPacket( const VxGUID sendToId, std::shared_ptr<VxSktBase>& sktBase, VxPktHdr* pktHdr, EPluginType overridePluginType = ePluginTypeInvalid ) = 0;

};

class AutoXferLock
{
public:
    AutoXferLock( VxMutex& mutex ) 
        : m_Mutex(mutex)	
    { 
        m_Mutex.lock(); 
    }

    ~AutoXferLock()
    { 
        m_Mutex.unlock(); 
    }

    VxMutex&						m_Mutex;
};