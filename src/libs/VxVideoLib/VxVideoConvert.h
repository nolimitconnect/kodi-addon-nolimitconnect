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

#include <CoreLib/VxDefs.h>

#include <memory>
#include <inttypes.h>

class CamImage
{
public:
	CamImage() = delete;
	CamImage( uint8_t* imgData, int dataLen, int imgWidth, int imgHeight, int camRotation )
		: m_DataLen( dataLen )
		, m_ImgWidth( imgWidth )
		, m_ImgHeight( imgHeight )
		, m_CamRotation( camRotation )
        , m_ImgData(imgData)
	{
	}

	
	int m_DataLen;
	int m_ImgWidth;
	int m_ImgHeight;
	int m_CamRotation;
	std::shared_ptr<uint8_t> m_ImgData;
};




//! convert image
// NOTE 1: caller must delete the returned buffer
uint8_t * VxConvertToJpg(	uint32_t	u32FourCcIn,		// FOURCC of format to convert
							uint8_t *	pu8DataIn,			// data to convert
							int			iDataLen,			// length of data 
							int 		iImageWidth,		// width of image in pixels
							int 		iImageHeight,		// height of image in pixels
							uint32_t&	u32RetDataLen,		// data length of jpg image
							int			iResizeToWidth = 0, // if not zero then resize to given length
							int			iResizeToHeight = 0 // if not zero then resize to given height
							);

//! convert image
// NOTE 1: caller must delete the returned buffer
uint8_t * VxConvertImage(	uint32_t		u32FourCcIn,		// FOURCC of format to convert
							uint8_t *		pu8DataIn,			// data to convert
							int 			iImageWidth,		// width of image in pixels
							int 			iImageHeight,		// height of image in pixels
							uint32_t		u32FourCcConverTo,	// FOURCC of format to convert to 
							uint32_t&		u32RetDataLen		// data length of converted image
							);

// calculate length of buffer required to hold image
uint32_t VxCalcImageDataLen(	uint32_t		u32FourCcIn,		// FOURCC of format
						int 	iImageWidth,		// width of image in pixels
						int 	iImageHeight );		// height of image in pixels

// calculate bytes per line of pixels.... assumes 4 byte boundary for RGB else pixels * bytes_per_pixels
uint32_t VxCalcImageStride(	uint32_t		u32FourCcIn,		// FOURCC of format
						int 	iImageWidth );		// width of image in pixels



