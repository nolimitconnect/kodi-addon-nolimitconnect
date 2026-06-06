//============================================================================
// Copyright (C) 2010 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "PluginBaseFileShare.h"
#include "PluginMgr.h"

#include <Plugins/FileInfo.h>
#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <PktLib/PktsFileShare.h>
#include <PktLib/PktsPluginOffer.h>
#include <PktLib/VxSearchDefs.h>
#include <PktLib/PktsFileInfo.h>
#include <PktLib/SearchParams.h>
#include <PktLib/VxCommon.h>

#include <CoreLib/VxFileUtil.h>
#include <CoreLib/VxFileShredder.h>
#include <CoreLib/VxGlobals.h>

#ifdef _MSC_VER
# pragma warning(disable: 4355) //'this' : used in base member initializer list
#endif

//============================================================================
PluginBaseFileShare::PluginBaseFileShare( P2PEngine& engine, PluginMgr& pluginMgr, VxNetIdent* myIdent, EPluginType pluginType, std::string fileInfoDbName )
    : m_FileInfoSharedFilesMgr( engine, *this, fileInfoDbName ) // TODO fix order of dependency
	, PluginBaseFiles( engine, pluginMgr, myIdent, pluginType, m_FileInfoSharedFilesMgr )
{
}

//============================================================================
void PluginBaseFileShare::onSharedFilesUpdated( uint16_t u16FileTypes )
{
	if( m_MyIdent->getSharedFileTypes() != u16FileTypes )
	{
		m_Engine.lockAnnouncePktAccess();
		m_MyIdent->setSharedFileTypes( (uint8_t)u16FileTypes );
		m_Engine.unlockAnnouncePktAccess();
		m_Engine.doPktAnnHasChanged( false );
	}
}
