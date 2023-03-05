/*
* This file should only be include by C/CPP files, as to not pollute the global namespace
*/

#pragma once

#include <crazygaze/microaudio/Core.h>

#define PROFILE()

// Returns the maximum of two values
/*
This macro returns the maximum from \a a and \a b .
Since this is a macro, be careful with the data types you use with it.
*/
#ifndef MAX
	#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

// Returns the minimum of two values
/*
This macro returns the minimum from \a a and \a b .
Since this is a macro, be careful with the data types you use with it.
*/
#ifndef MIN
	#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#endif

#define CLAMP(val, a, b) MAX(MIN(val,b),a)

#define INRANGE(val, low, high) (((val)>=(low))&&((val)<=(high)))
#define INBETWEEN(val, low, high) (((val)>(low))&&((val)<(high)))

// Disable deprecated CRT warnings in VC++
#if defined(_MSC_VER) && (_MSC_VER > 1300)
	#pragma warning( disable : 4996 )
#endif

//
// Log macros
//
#if CZMICROAUDIO_LOG_ENABLED
	#define CZMICROAUDIO_LOG Core::get()->onLog
#else
	#define CZMICROAUDIO_LOG (void)sizeof
#endif

//
// Error callback macros
//
#if CZMICROAUDIO_ERRORCALLBACK_ENABLED
	#define CZMICROAUDIO_ERRORCALLBACK(err) Core::get()->onError(err)
#else
	#define CZMICROAUDIO_ERRORCALLBACK(err) ((void)0)
#endif

//
// Error macros
//
#define CZMICROAUDIO_RET_ERROR(err) \
	{ \
		CZMICROAUDIO_ERRORCALLBACK(err); \
		CZMICROAUDIO_LOG(LogLevel::Error, "ERROR %d at %s:%s:%d (%s)\n",err, __FILE__, __FUNCTION__, __LINE__, getErrorMsg(err)); \
		return err; \
	}
#define CZMICROAUDIO_RETNULL(err) \
	{ \
		CZMICROAUDIO_ERRORCALLBACK(err); \
		CZMICROAUDIO_LOG(LogLevel::Error, "ERROR %d at %s:%s:%d (%s)\n",err, __FILE__, __FUNCTION__, __LINE__, getErrorMsg(err)); \
		return NULL; \
	}

//
// Assert macros
//
#if CZMICROAUDIO_DEBUG
	#define CZMICROAUDIO_ASSERT(expr)                                                          \
	if (bool res = (expr); !res)                                                               \
	{                                                                                          \
		CZMICROAUDIO_LOG(LogLevel::Error, "ASSERT failed at %s:%d (%s)\n",__FILE__, __LINE__); \
		assert(res);                                                                           \
	}

	// Difference between this and assert is this always evaluates the expression
	#define CZMICROAUDIO_CHECK(expr) CZMICROAUDIO_ASSERT(expr)

#else

	#define CZMICROAUDIO_ASSERT(expr) ((void)0)
	// Difference between this and assert is this always evaluates the expression
	#define CZMICROAUDIO_CHECK(expr) expr

#endif

