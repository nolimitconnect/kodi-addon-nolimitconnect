#pragma once
//============================================================================
// Copyright (C) 2018 Brett R. Jones
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include "IDefs.h"

class INlcEvents
{
public:
    virtual void                fromGuiKeyPressEvent( EMediaModule mediaModule, int key, int mod ) = 0;
    virtual void                fromGuiKeyReleaseEvent( EMediaModule mediaModule, int key, int mod ) = 0;

    virtual void                fromGuiMousePressEvent( EMediaModule mediaModule, int mouseXPos, int mouseYPos, int mouseButton ) = 0;
    virtual void                fromGuiMouseReleaseEvent( EMediaModule mediaModule, int mouseXPos, int mouseYPos, int mouseButton ) = 0;
    virtual void                fromGuiMouseMoveEvent( EMediaModule mediaModule, int mouseXPos, int mouseYPos ) = 0;

	//virtual void                fromGuiResizeBegin( EMediaModule mediaModule, int winWidth, int winHeight ) = 0;
	//virtual void                fromGuiResizeEvent( EMediaModule mediaModule, int winWidth, int winHeight ) = 0;
	//virtual void                fromGuiResizeEnd( EMediaModule mediaModule, int winWidth, int winHeight ) = 0;
    //virtual void                fromGuiRenderWindowResize( EMediaModule mediaModule, int winWidth, int winHeight ) = 0;

    virtual void                fromGuiCloseEvent( EMediaModule mediaModule ) = 0;
    virtual void                fromGuiVisibleEvent( EMediaModule mediaModule, bool isVisible ) = 0;

};
