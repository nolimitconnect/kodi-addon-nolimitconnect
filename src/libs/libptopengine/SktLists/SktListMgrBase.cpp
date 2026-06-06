//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "SktListMgrBase.h"
#include <P2PEngine/P2PEngine.h>

//============================================================================
SktListMgrBase::SktListMgrBase( P2PEngine& engine )
    : m_Engine( engine )
{
}

//============================================================================
void SktListMgrBase::onUpdateIdent( VxGUID& onlineId, int64_t timestamp )
{
    m_Engine.getToGui().toGuiIndentListUpdate( m_ListType, onlineId, timestamp );
}

//============================================================================
void SktListMgrBase::onRemoveIdent( VxGUID& onlineId )
{
    m_Engine.getToGui().toGuiIndentListRemove( m_ListType, onlineId );
}