#include "VxJpgLib.h"

/*
#include <stdio.h>
extern "C" { 
#include "jpeglib.h" 
}
#include <setjmp.h>
#include "VxJpgLib.h"
#include "stdlib.h"
#include "stdio.h"
#include "memory.h"

struct jpg_datastore{
  unsigned char *data;
  size_t len;

  jpg_datastore () { data = 0; len = 0; }
  ~jpg_datastore () { free (data); }
};
*/

#include <turbojpeg.h>

#include "memory.h"

tjhandle GetJpegCompressorInstance( void )
{
    static tjhandle jpegCompressor = nullptr;
    if(!jpegCompressor)
    {
        jpegCompressor = tjInitCompress();
    }

    return jpegCompressor;
}

int32_t VxBmp2Jpg(	int				iBitsPerPixel,	//number of bits each pixel..(For now must be 24)
                      unsigned char *   pu8Bits,		//bits of bmp to convert
                      int				width,			//width of image in pixels
                      int				height,		//height of image in pixels
                      int				iQuality,		//quality of image
                      int				iJpgBufLen,		//maximum length of pu8RetJpg
                      unsigned char *   pu8RetJpg,		//buffer to return Jpeg image
                      long *			ps32RetJpgLen ) //return length of jpeg image
{
	
	* ps32RetJpgLen = 0;


    long unsigned int _jpegSize = iJpgBufLen;
    tjhandle _jpegCompressor = GetJpegCompressorInstance();

    int rc = tjCompress2( _jpegCompressor, pu8Bits, width, 0, height, TJPF_RGB,
                          &pu8RetJpg, &_jpegSize, TJSAMP_420, iQuality,
                          TJFLAG_FASTDCT | TJFLAG_NOREALLOC );

    if( rc == 0 && _jpegSize )
    {
        *ps32RetJpgLen = _jpegSize;
    }


	/* And we're done! */
	return 0;
}
