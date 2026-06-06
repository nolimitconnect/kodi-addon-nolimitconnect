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

#include <CoreLib/VxGUID.h>

class PktBlobEntry;

#pragma pack(push)
#pragma pack(1)

class PluginId
{
public:
	PluginId() = default;
    PluginId( VxGUID& onlineId, EPluginType pluginType );
    PluginId( const PluginId& rhs );
    //do not use virtuals because this object is packed in packets
	PluginId&				    operator =( const PluginId& rhs );
    bool						operator == ( const PluginId& rhs ) const;
    bool						operator != ( const PluginId& rhs ) const;
    bool						operator < ( const PluginId& rhs ) const;
    bool						operator <= ( const PluginId& rhs ) const;
    bool						operator > ( const PluginId& rhs ) const;
    bool						operator >= ( const PluginId& rhs ) const;

    bool                        addToBlob( PktBlobEntry& blob );
    bool                        extractFromBlob( PktBlobEntry& blob );

    void						setOnlineId( VxGUID& onlineId )             { m_OnlineId = onlineId; }
    VxGUID&					    getOnlineId( void )                         { return m_OnlineId; }

    void						setPluginType( enum EPluginType pluginType )     { m_PluginType = (uint8_t)pluginType; }
    EPluginType  				getPluginType( void ) const                 { return (EPluginType)m_PluginType; }

    // returns 0 if equal else -1 if less or 1 if greater
    int							compareTo( PluginId& guid );
    // returns true if guids are same value
    bool						isEqualTo( const PluginId& guid );
    // get a description of the plugin id
    std::string                 describePluginId( void ) const;

protected:
	//=== vars ===//
    VxGUID					    m_OnlineId;
    uint8_t					    m_PluginType{ 0 };
};

#pragma pack(pop)
