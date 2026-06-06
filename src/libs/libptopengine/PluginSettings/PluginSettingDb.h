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

#include <GuiInterface/IDefs.h>

#include <CoreLib/DbBase.h>

#include <vector>

#define PLUGIN_SETTING_DB_VERSION 1

class PluginSetting;

class PluginSettingDb : public DbBase
{
public:
    PluginSettingDb();
    virtual ~PluginSettingDb() = default;

    bool						isValid( void )							{ return m_bIsValid; }
    void						setIsValid( bool bValid )				{ m_bIsValid = bValid; }

    //=== overrides ===//
    //! delete tables from database 
    virtual int32_t				onDeleteTables( int oldVersion );
    //! create tables in database 
    virtual int32_t				onCreateTables( int iDbVersion );

    bool                        updatePluginSetting( EPluginType pluginType, PluginSetting& pluginSetting );
    bool                        getPluginSetting( EPluginType pluginType, PluginSetting& pluginSetting );
    bool                        getAllPluginSettings( std::vector<PluginSetting>& pluginSettingList );

protected:
    bool                        pluginExistsInTable( std::string pluginName, std::string& strTableName );

    //=== vars ===//
    bool						m_bIsValid;
};


PluginSettingDb& GetPluginSettingDbInstance();