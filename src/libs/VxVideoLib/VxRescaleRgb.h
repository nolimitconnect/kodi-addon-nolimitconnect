#pragma once
//============================================================================
// Copyright (C) 2003 Brett R. Jones 
//
// Code copyrighted by Brett R. Jones is under dual license similar to Ruby's license
// See file COPYING and LEGAL in root of the No Limit Connect project
//
// bjones.engineer@gmail.com
// https://nolimitconnect.com
//============================================================================

//! resize rgb 24 bit image
// NOTE 1: caller must delete the returned buffer
uint8_t * VxResizeRgbImage(	uint8_t *	pu8VidData, 
						int		iImageWidthIn, 
						int		iImageHeightIn, 
						int		iResizeToWidth, 
						int		iResizeToHeight,
						int		iRotation );

// NOTE 1: caller must delete the returned buffer
uint8_t * VxRotateRgbImage(		uint8_t *	pu8VidData, 
							int		iImageWidthIn, 
							int		iImageHeightIn, 
							int		iRotateAngle );
