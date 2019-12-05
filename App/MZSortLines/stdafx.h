// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <Windows.h>
#include <assert.h>

//#include "zSTLDef.h"

#include <string>

#ifndef STL_string
  #define STL_string std::string
  #ifndef _UNICODE
    typedef STL_string	 STLString;
  #endif
#endif

#ifndef STL_wstring
  #define STL_wstring std::wstring
  #ifdef _UNICODE
    typedef STL_wstring  STLString;
  #endif
#endif


