//============================================================================
// Copyright (C) 2015 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <P2PEngine/P2PEngine.h>
#include <GuiInterface/IToGui.h>

#include <BlobXferMgr/BlobInfo.h>

//============================================================================
void P2PEngine::callbackBlobAdded( BlobInfo* blobInfo )
{
    IToGui::getIToGui().toGuiBlobAdded( blobInfo );
}

//============================================================================
void P2PEngine::callbackBlobRemoved( BlobInfo* blobInfo )
{
    IToGui::getIToGui().toGuiBlobAction( eAssetActionRemoveFromAssetMgr, blobInfo->getAssetUniqueId(), 0 );
}

//============================================================================
void P2PEngine::callbackBlobHistory( BlobInfo* blobInfo )
{
	IToGui::getIToGui().toGuiBlobSessionHistory( blobInfo );
}
