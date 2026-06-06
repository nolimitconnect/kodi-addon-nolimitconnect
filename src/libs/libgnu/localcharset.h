/* Determine a canonical name for the current locale's character encoding.
   Copyright (C) 2000-2003, 2009-2016 Free Software Foundation, Inc.
   This file is part of the GNU CHARSET Library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, see <http://www.gnu.org/licenses/>.  */

#pragma once
#include <libgnu/config_libgnu.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Determine the current locale's character encoding, and canonicalize it
   into one of the canonical names listed in config.charset.
   The result must not be freed; it is statically allocated.
   If the canonical name cannot be determined, the result is a non-canonical
   name.  */
extern const char * locale_charset (void);

#undef DCHAR_CONV_FROM_ENCODING
#define DCHAR_CONV_FROM_ENCODING dchar_conv_from_encoding_gnu

// Handling of unconvertible characters.
// enum iconv_ilseq_handler
// {
//     iconveh_error,                /* return and set errno = EILSEQ */
//     iconveh_question_mark,        /* use one '?' per unconvertible character */
//     iconveh_escape_sequence       /* use escape sequence \uxxxx or \Uxxxxxxxx */
// };

// caller must free the returned encoded string or will have memory leak
extern char * dchar_conv_from_encoding_gnu(const char *toEncoding, int howHandleUnconvertableChars,
                                         const char *fromString, size_t fromCharsLen, void* notUsed1,
                                         void* notUsed2,
                                         size_t* retEncodedLen);

// caller must free the returned encoded string or will have memory leak
extern char * convert_encoding_gnu(const char *textToConvert, const char *fromEncoding, const char *toEncoding);

#ifdef __cplusplus
}
#endif

