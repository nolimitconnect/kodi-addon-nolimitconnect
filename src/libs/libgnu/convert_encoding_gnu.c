#include "localcharset.h"

#include <CoreLib/VxDebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

#ifndef MAX
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#endif

//============================================================================
char * convert_encoding(const char *input, const char *from_encoding, const char *to_encoding)
{
    iconv_t cd = iconv_open(to_encoding, from_encoding);
    if (cd == (iconv_t)(-1))
    {
        LogMsg( LOG_ERROR, "%s iconv_open", __func__ );
        return NULL;
    }

    size_t inbytesleft = strlen(input);
    size_t outbytesleft = inbytesleft * 4; // Allocate enough space for output
    char *output = malloc(outbytesleft);
    if (!output)
    {
        LogMsg( LOG_ERROR, "%s malloc", __func__ );
        iconv_close(cd);
        return NULL;
    }

    char *outbuf = output;
    size_t result = iconv(cd, &input, &inbytesleft, &outbuf, &outbytesleft);

    if (result == (size_t)(-1))
    {
        LogMsg( LOG_ERROR, "%s iconv", __func__ );
        free(output);
        iconv_close(cd);
        return NULL;
    }

    // Null-terminate the output string
    *outbuf = '\0';
    LogMsg( LOG_VERBOSE, "%s Converted string: %s", __func__, output);
    //free(output);
    iconv_close(cd);
    return output;
}

//============================================================================
char * dchar_conv_from_encoding_gnu(const char *toEncoding, int howHandleUnconvertableChars,
                                         const char *fromString, size_t fromCharsLen, void* notUsed1,
                                         void* notUsed2,
                                         size_t* retEncodedLen)
{
    // nlc only uses ascii and utf8
    // DCHAR_CONV_FROM_ENCODING is only used in vasnprintf here in libgnu and in libasprintf
    // therefore we can just make a copy

    // if in the future we need to do actual conversion then we can use the
    // above function convert_encoding

    int strLen = MAX(fromCharsLen, strlen(fromString) );

    char* retConverted = malloc(strLen + 4);
    strcpy(retConverted, fromString);

    return retConverted;
}
