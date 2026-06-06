//============================================================================
// Copyright (C) 2019 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginSettingDb.h"
#include "PluginSetting.h"

#include <CoreLib/sqlite3.h>
#include <CoreLib/BinaryBlob.h>
#include <CoreLib/VxDebug.h>
#include <PktLib/VxCommon.h>

#include <stdio.h>

namespace
{
    std::string 		DATABASE_NAME = "plugin_setting.db";

    std::string 		TABLE_PLUGIN_SETTING = "plugin_setting";

    std::string 		COLUMNS_PLUGIN_SETTING = "plugin_name,setting_blob";
    std::string 		CREATE_COLUMNS_PLUGIN_SETTING = " (plugin_name TEXT PRIMARY KEY, setting_blob BLOB ) ";
}

//============================================================================
PluginSettingDb& GetPluginSettingDbInstance()
{
    static PluginSettingDb g_PluginSettingDb;
    return g_PluginSettingDb;
}

//============================================================================
PluginSettingDb::PluginSettingDb()
    : DbBase( "PluginSettingDb" )
{
}

//============================================================================
//! create tables in database 
int32_t PluginSettingDb::onCreateTables( int iDbVersion )
{
    std::string exeStr = "CREATE TABLE " + TABLE_PLUGIN_SETTING + CREATE_COLUMNS_PLUGIN_SETTING;
    int32_t rc = sqlExec( exeStr );
    return rc;
}

//============================================================================
// delete tables in database
int32_t PluginSettingDb::onDeleteTables( int oldVersion )
{
    int32_t rc = sqlExec( "DROP TABLE IF EXISTS plugin_setting" );
    return rc;
}

//============================================================================
//! return true if online id exists in table
bool PluginSettingDb::pluginExistsInTable( std::string pluginName, std::string& strTableName )
{
    bool bExists = false;
    DbCursor * cursor = startQueryInsecure( "SELECT * FROM %s WHERE plugin_name='%s'", strTableName.c_str(), pluginName.c_str() );
    if( NULL != cursor )
    {
        if( cursor->getNextRow() )
        {
            bExists = true;
        }

        cursor->close();
    }

    return bExists;
}

//=========================================================================
// plugin settings
//=========================================================================
//============================================================================
bool PluginSettingDb::updatePluginSetting( EPluginType pluginType, PluginSetting& pluginSetting )
{
    bool result = false;
    BinaryBlob settingBinary;
    if( pluginSetting.toBinary( settingBinary ) )
    {
        std::string strPluginName = GetPluginName( pluginType );
        if( !strPluginName.empty() && settingBinary.getBlobLen() )
        {
            DbBindList bindList( strPluginName.c_str() );
            sqlExec( "DELETE FROM plugin_setting WHERE plugin_name=?", bindList );
            bindList.add( settingBinary.getBlobData(), settingBinary.getBlobLen() );

            int32_t rc = sqlExec( "INSERT INTO plugin_setting (plugin_name,setting_blob) values(?,?)", bindList );
            if( rc )
            {
                LogMsg( LOG_ERROR, "PluginSettingDb::updatePluginSetting: ERROR %d %s", rc, sqlite3_errmsg( m_Db ) );
            }
            else
            {
                result = true;
            }
        }
        else
        {
            LogMsg( LOG_ERROR, "PluginSettingDb::updatePluginSetting: Invalid params" );
        }
    }

    return result;
}

//============================================================================
bool PluginSettingDb::getPluginSetting( EPluginType pluginType, PluginSetting& pluginSetting )
{
    bool result = false;
    std::string pluginName = GetPluginName( pluginType );
    if( !pluginName.empty() && pluginExistsInTable( pluginName, TABLE_PLUGIN_SETTING ) )
    {
        DbCursor * cursor = startQueryInsecure( "SELECT setting_blob FROM plugin_setting WHERE plugin_name='%s'", pluginName.c_str() );
        if( NULL != cursor )
        {
            if( cursor->getNextRow() )
            {
                int iBlobLen = 0;
                uint8_t* settingData = ( uint8_t* )cursor->getBlob( 1, &iBlobLen );
                if( settingData && iBlobLen > 1 && iBlobLen <= BLOB_PLUGIN_SETTING_MAX_STORAGE_LEN )
                {
                    BinaryBlob pluginBlob;
                    pluginBlob.setBlobData( settingData, iBlobLen, true, true );
                    if( pluginSetting.fromBinary( pluginBlob ) )
                    {
                        result = true;
                    }
                }
                else
                {
                    LogMsg( LOG_ERROR, "PluginSettingDb::getPluginSettings: incorrect blob len in db.. was code changed????\n" );
                    cursor->close();
                    // remove the invalid blob
                    DbBindList bindList( pluginName.c_str() );
                    int32_t rc = sqlExec( "DELETE FROM plugin_setting WHERE plugin_name=?", bindList );
                    if( rc )
                    {
                        LogMsg( LOG_ERROR, "PluginSettingDb::getPluginSettings: could not remove plugin by name %s\n", pluginName.c_str() );
                    }

                    return false;
                }
            }

            cursor->close();
        }

    }

    return result;
}

//============================================================================
bool PluginSettingDb::getAllPluginSettings( std::vector<PluginSetting>& settingList )
{
    settingList.clear();

    bool bResult = false;
    DbCursor * cursor = startQueryInsecure( "SELECT * FROM plugin_setting" );
    if( NULL != cursor )
    {
        while( cursor->getNextRow() )
        {
            std::string pluginName = cursor->getString( 0 );
            if( !pluginName.empty() )
            {
                int iBlobLen = 0;
                uint8_t* settingData = ( uint8_t* )cursor->getBlob( 1, &iBlobLen );
                if( settingData && iBlobLen > 1 && iBlobLen <= BLOB_PLUGIN_SETTING_MAX_STORAGE_LEN )
                {
                    BinaryBlob pluginBlob;
                    pluginBlob.setBlobData( settingData, iBlobLen, true, true );
                    PluginSetting pluginSetting;
                    if( pluginSetting.fromBinary( pluginBlob ) )
                    {
                        settingList.push_back( pluginSetting );
                        bResult = true;
                    }
                }
                else
                {
                    LogMsg( LOG_ERROR, "PluginSettingDb::getAllPluginSettings: incorrect blob len in db.. was code changed????\n" );
                    cursor->close();
                    // remove the invalid blob
                    DbBindList bindList( pluginName.c_str() );
                    int32_t rc = sqlExec( "DELETE FROM plugin_setting WHERE plugin_name=?", bindList );
                    if( rc )
                    {
                        LogMsg( LOG_ERROR, "PluginSettingDb::getAllPluginSettings: could not remove plugin by name %s\n", pluginName.c_str() );
                    }

                    return false;
                }
            }
            else
            {
                LogMsg( LOG_ERROR, "PluginSettingDb::getAllPluginSettings: null plugin name\n" );
            }
        }

        cursor->close();
    }


    return bResult && !settingList.empty();
}


