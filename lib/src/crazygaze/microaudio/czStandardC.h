//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Wrappers or czPlayer own implementations of missing ANSI C functions, which may not be supported on some platforms
//

#ifndef _CZSTANDARDC_H
#define _CZSTANDARDC_H

#include "czPlayerConfig.h"

// This should exist in all platforms
#include <stdlib.h>

// Define assert
#if (CZ_PLATFORM==CZ_PLATFORM_WIN32) || (CZ_PLATFORM==CZ_PLATFORM_SYMBIAN)
	#include <assert.h>
#endif

// string functions
#if (CZ_PLATFORM==CZ_PLATFORM_WIN32) || (CZ_PLATFORM==CZ_PLATFORM_SYMBIAN)
	#include <string.h>
#endif

// variable arguments, and vsprintf is only needed if we have logging enabled
#if CZ_PLAYER_LOG_ENABLED
	#include <stdarg.h>
	#include <stdio.h>
#endif


// Math
#if (CZ_PLATFORM==CZ_PLATFORM_WIN32) || (CZ_PLATFORM==CZ_PLATFORM_SYMBIAN)
	#include <math.h>
#endif

#endif
