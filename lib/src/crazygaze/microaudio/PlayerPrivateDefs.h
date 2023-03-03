//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// This files declares stuff needed only by czPlayer internals, which the user doesn't need to know about
//

#ifndef _CZPLAYERPRIVATEDEFS_H_
#define _CZPLAYERPRIVATEDEFS_H_


#define PROFILE()

// Returns the maximum of two values
/*
This macro returns the maximum from \a a and \a b .
Since this is a macro, be careful with the data types you use with it.
*/
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Returns the minimum of two values
/*
This macro returns the minimum from \a a and \a b .
Since this is a macro, be careful with the data types you use with it.
*/
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

#define CLAMP(val, a, b) MAX(MIN(val,b),a)

#define INRANGE(val, low, high) (((val)>=(low))&&((val)<=(high)))
#define INBETWEEN(val, low, high) (((val)>(low))&&((val)<(high)))

//#define INRANGE(val,low,high)	(((val)>(high))?0:(((val)<(low))?0:1))
//#define INBETWEEN(val,low,high)	(((val)>=(high))?0:(((val)<=(low))?0:1))


// Disable deprecated CRT warnings in VC++
#if defined(_MSC_VER) && (_MSC_VER > 1300)
	#pragma warning( disable : 4996 )
#endif

#include <crazygaze/microaudio/Player.h>

#include <string.h>


#if CZMICROAUDIO_IT_ENABLED
	typedef __int64 TInt64;
	#define TInt64_Lsr(v,bits) v>>=bits
	#define TInt64_GetTInt(v) int(v)
#endif

//
// Parent object to use for macros to access the core. A macro its used, so I can redefine it whenever needed, and change the way to access in any place
//
#define COREOBJ m_core

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
	#define CZERRORCALLBACK(err) Core::get()->onError(err)
#else
	#define CZERRORCALLBACK(err) ((void)0)
#endif


//
// Error macros
//
#define CZERROR(err) \
	{ \
		CZERRORCALLBACK(err); \
		CZMICROAUDIO_LOG(LogLevel::Error, "ERROR %d at %s:%s:%d (%s)\n",err, __FILE__, __FUNCTION__, __LINE__, GetErrorMsg(err)); \
		return err; \
	}
#define CZERROR_RETNULL(err) \
	{ \
		CZERRORCALLBACK(err); \
		CZMICROAUDIO_LOG(LogLevel::Error, "ERROR %d at %s:%s:%d (%s)\n",err, __FILE__, __FUNCTION__, __LINE__, GetErrorMsg(err)); \
		return NULL; \
	}

//
// Assert macros
//
#if CZ_DEBUG
	#define CZASSERT(expr) { bool exp_res=(expr); if(!(exp_res)) CZMICROAUDIO_LOG(LogLevel::Error, "ASSERT failed at %s:%d (%s)\n",__FILE__, __LINE__); assert(exp_res);  }
	// Difference between this and assert is this always evaluates the expression
	#define CZCHECK(expr) CZASSERT(expr)
#else
	#define CZASSERT(expr) ((void)0)
	// Difference between this and assert is this always evaluates the expression
	#define CZCHECK(expr) expr
#endif

//
// Placement new/delete, with dummy enums, to differentiate from any other new/delete
enum etOperatorPlacementNewDummy
{
	kOperatorPlacementNewDummy
};
inline void* operator new(size_t, void* addr, const char*, int, const char*, etOperatorPlacementNewDummy)
{
	return addr;
}
inline void* operator new(size_t, void* addr, etOperatorPlacementNewDummy)
{
	return addr;
}
inline void operator delete( void* ptr, void*, etOperatorPlacementNewDummy)
{
}
inline void operator delete( void* ptr, void*, const char*, int, const char*, etOperatorPlacementNewDummy)
{
}

/*
#if CZ_DEBUG
	#define CZCHECKPOINT COREOBJ->CheckPoint(__FILE__, __FUNCTION__, __LINE__);
#endif
*/

// to cut down on compiler warnings or unused paramaters
#define CZUNUSED(argname) /* argname */

#endif

