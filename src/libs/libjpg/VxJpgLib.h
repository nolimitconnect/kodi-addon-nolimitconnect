#pragma once

#include <stdint.h>

int32_t VxWriteJpg(	const char *	pFileName,
					int				iBitsPerPixel,	//number of bits each pixel..(For now must be 24)
					unsigned char * pu8Bits,		//bits of bmp to convert
					int				iWidth,			//width of image in pixels
					int				iHeight,		//height of image in pixels
					int				iQuality );		//quality of image

int32_t VxBmp2Jpg(	int				iBitsPerPixel,	//number of bits each pixel..(For now must be 24)
					unsigned char * pu8Bits,		//bits of bmp to convert
					int				iWidth,			//width of image in pixels
					int				iHeight,		//height of image in pixels
					int				iQuality,		//quality of image
					int				iJpgBufLen,		//maximum length of pu8RetJpg
					unsigned char * pu8RetJpg,		//buffer to return Jpeg image
					long *			ps32RetJpgLen ); //return length of jpeg image


int32_t VxJpg2Bmp(	unsigned char * pu8Bits,		// bits of jpg to convert
					unsigned long	u32DataLen,		// length of jpg data
					int				iWidth,			// width of image in pixels
					int				iHeight,		// height of image in pixels
					unsigned char * pu8RetBmp,		// buffer to return Bmp Data
					long *			ps32RetBmpLen ); //return length of Bmp data



