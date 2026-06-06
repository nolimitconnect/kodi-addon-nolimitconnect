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

#include "VxDebug.h"

#include <string>

enum EXmlParseType
{
	eXmlParseError,
	eXmlStartTag,
	eXmlEndTag,
	eXmlEmptyTag,
	eXmlDeclarationTag,
	eXmlStringTag,
	eXmlAttibuteTag,
	eXmlCommentTag,
	eXmlUnknownContentTag,
	eXmlDocEnd
};

// callback example void XmlCallback( void * pvUserData, EXmlParseType eXmlType, const char* pName, const char* pValue );

typedef void 	(*VX_XML_CALLBACK_T)( void *, EXmlParseType,  char const *, char const * );

bool XmlIsSpace(char c);
char XmlTolower(char c);
bool XmlStringNoCaseBeginsWith(char const* s1, char const* s2);
void XmlCopyToLower( std::string& strDest, char const* pSrc );
bool XmlStringNoCaseEqual(char const* s1, char const* s2);
void VxXmlParse( void * pvUserData, char* pStart, char* pEnd, VX_XML_CALLBACK_T callback );
