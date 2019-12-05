//////////////////////////////////////////////////////////////////////////
//
// File : zSTLDef.h
//
// Version     : 1.0 ( 20-Okt-2005 )
// Description : Setup up STL macros. Including STL and using the macros that are setup here will 
//               make your project eaiser to work with different STL implmentations. only do the changes in here
//
//    Author : Mathias Svensson  ( http://www.result42.com )
// Copyright : 2005,2006 - Mathias Svensson
//   Licence : Free to use and modify as long this header is left as is.
//
//  Warranty : None what so ever. Use at own risk.
//
//////////////////////////////////////////////////////////////////////////

//  Do not use "using namespace std" if you really need to. 
//  It will often create more problems that it solves.


// Uncomment the one you do not need
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#if _MSC_VER >= 1700 // vc11+
	#include <memory>
#endif

/*
#include <list>
#include <hash_map>
#include <stack>
#include <queue>
#include <set>
*/
// MS STL 
#if _MSC_VER < 1300
#define STL_HASHMAP std::hash_map
#else
#define STL_HASHMAP stdext::hash_map
#endif

#define STL_MAP std::map
#define STL_MULTIMAP std::multimap
#define STL_VECTOR std::vector
#define STL_LIST std::list
#define STL_SET std::set
#define STL_SLIST std::slist
#define STL_PAIR std::pair
#define STL_string std::string
#define STL_wstring std::wstring
#define STL_STACK std::stack
#define STL_QUEUE std::queue
#define STL_DEQUE std::deque
#define STL_PRIOQUEUE std::priority_queue

#define STL_reverse std::reverse
#define STL_copy std::copy
#define STL_find std::find
#define STL_STD std::


#ifdef _UNICODE
typedef STL_wstring  STLString;
#else
typedef STL_string	 STLString;
#endif

