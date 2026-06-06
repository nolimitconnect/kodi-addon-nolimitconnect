//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================
#pragma once

#include "PluginSetting.h"

#include <CoreLib/VxMutex.h>

class PluginSettingDb;
class P2PEngine;

class PluginSettingMgr 
{
public:
    PluginSettingMgr( P2PEngine& engine );
    virtual ~PluginSettingMgr() = default;

    bool                        setPluginSetting( PluginSetting& pluginSetting, int64_t modifiedTimeMs = 0 );
    bool                        getPluginSetting( EPluginType pluginType, PluginSetting& pluginSetting );
    bool                        getHostTitleAndDescription( EPluginType pluginType, std::string& hostTitle, std::string& hostDesc, int64_t& lastModifiedTime );

protected:
    bool                        initPluginSettingMgr( void );
    PluginSettingDb&            getPluginSettingDb() { return m_PluginSettingDb; }

    //=== vars ===//
    P2PEngine&					m_Engine;

    bool                        m_SettingMgrInitied = false;
    VxMutex                     m_SettingMutex;
    PluginSettingDb&            m_PluginSettingDb;
    std::vector<PluginSetting>  m_SettingList;
};

