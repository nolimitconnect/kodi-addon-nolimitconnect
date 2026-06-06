//============================================================================
// Copyright (C) 2021 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license 
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "IdentListMgrBase.h"
#include <P2PEngine/P2PEngine.h>

//============================================================================
IdentListMgrBase::IdentListMgrBase( P2PEngine& engine )
    : m_Engine( engine )
{
}

//============================================================================
void IdentListMgrBase::onUpdateIdent( VxGUID& onlineId, int64_t timestamp )
{
    m_Engine.getToGui().toGuiIndentListUpdate( m_ListType, onlineId, timestamp );
}

//============================================================================
void IdentListMgrBase::onRemoveIdent( VxGUID& onlineId )
{
    m_Engine.getToGui().toGuiIndentListRemove( m_ListType, onlineId );
}