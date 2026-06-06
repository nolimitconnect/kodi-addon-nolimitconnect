#pragma once
//============================================================================
// Copyright (C) 2013 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

#include <NlcDependLibrariesConfig.h>

// from fourcc.org 
#define FOURCC_RGB		0x32424752
#define FOURCC_RGBA		0x41424752
#define FOURCC_I420		0x30323449
#define FOURCC_YUY2		0x32595559
#define FOURCC_YV12		0x32315659
#define FOURCC_IYUV		0x56555949

#define FOURCC_YUVS		0x12345678 // I made this special for android YUV 4.2.0 SP
// made up to handle reversed blue and red on windows
#define FOURCC_BGR		0x32524742
#define FOURCC_BGRA		0x52474241

#ifndef VIDCAP_HWND
	#define VIDCAP_HWND void *
#endif //int32_t

#define VX_VIDCAP_BASE_ERR				1549088

#define VX_VIDCAP_NO_DEVICES			VX_VIDCAP_BASE_ERR + 1
#define VX_VIDCAP_DRIVER_ERR			VX_VIDCAP_BASE_ERR + 2


//forward declare
class IVxVidCap;

//! return interface to video capture
IVxVidCap * VxGetVidCapInterface( void );

//! implements video capture
class IVxVidCap
{
public:
	IVxVidCap() = default;
	virtual ~IVxVidCap() = default;
	//! startup return number of video sources ( iVidSrcNum is zero to number of Video Sources - 1 )
	virtual int				startupVidCap( int iPreferedImageWidth = 640, int iPreferedImageHeight = 480 ) = 0;
	//! shutdown
	virtual void			shutdownVidCap( void ) = 0;

	//! start preview 
	virtual bool			startPreview(		VIDCAP_HWND	pvHwnd,				// handle of window to preview video on	
												int			iVidSrcNum	= 0,	// video source to enable ( 0 = default )
												int			iXPos		= 0,	// x pos to draw at
												int			iYPos		= 0,	// y pos to draw at
												int			iWidth		= 0,	// width of preview window
												int			iHeight		= 0,	// height of preview window
												int			iFps		= 30	// preview frame rate
												) = 0;
	//! stop preview 
	virtual bool			stopPreview( int iVidSrcNum ) = 0;

	//! take snap shot 
	// NOTE 1: caller must delete the returned buffer
	// NOTE 2: all formats are raw data except FOURCC_RGB which returns complete bitmap
	virtual uint8_t *			takeSnapShot( int iVidSrcNum, uint32_t& u32RetImageLen, uint32_t& u32RetFormat ) = 0;

};




