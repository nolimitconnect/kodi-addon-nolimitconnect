#pragma once

#include <stdint.h>

int32_t VxBmp2Jpg(	int				iBitsPerPixel,	//number of bits each pixel..(For now must be 24)
					unsigned char * pu8Bits,		//bits of bmp to convert
					int				iWidth,			//width of image in pixels
					int				iHeight,		//height of image in pixels
					int				iQuality,		//quality of image
					int				iJpgBufLen,		//maximum length of pu8RetJpg
					unsigned char * pu8RetJpg,		//buffer to return Jpeg image
					long *			ps32RetJpgLen ); //return length of jpeg image






