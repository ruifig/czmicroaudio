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

#include "czPlayer.h"

// Include specific platform OS headers
#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN
	#include <E32Base.h>
	#ifndef UIQ_BUILD
	#include <AknUtils.h>
	#endif
	#include <e32svr.h>
	#include <MdaAudioOutputStream.h>
	#include <mda\common\audio.h>
#endif

#if CZ_PLAYER_IT_ENABLED
	
	#if CZ_PLATFORM==CZ_PLATFORM_SYMBIAN
		#define TInt64_Lsr(v,bits) v.Lsr(bits)
		#define TInt64_GetTInt(v) v.GetTInt()
	#elif CZ_PLATFORM==CZ_PLATFORM_WIN32
		typedef __int64 TInt64;
		#define TInt64_Lsr(v,bits) v>>=bits
		#define TInt64_GetTInt(v) int(v)
	#endif
#endif


//
// Parent object to use for macros to access the core. A macro its used, so I can redefine it whenever needed, and change the way to access in any place
//
#define COREOBJ m_core

//
// Log macros
//
#if CZ_PLAYER_LOG_ENABLED
	#define CZLOG COREOBJ->OnLog
#else
	#define CZLOG (void)sizeof
#endif




//
// Memory allocation macros
//
#if CZ_DEBUG
	#define CZALLOC(size) COREOBJ->AllocMem(size,  __FILE__, __LINE__)
	#define CZFREE(ptr) COREOBJ->FreeMem(ptr, __FILE__, __LINE__)
	#define CZNATIVEALLOC(size) ::cz::AllocNativeMem(size,  __FILE__, __LINE__)
	#define CZNATIVEFREE(ptr) ::cz::FreeNativeMem(ptr, __FILE__, __LINE__)
	#define CZNEW(CZOBJECTTYPE) new(COREOBJ, __FILE__, __LINE__) CZOBJECTTYPE
	#define CZDELETE(CZOBJECT) delete CZOBJECT
#else
	#define CZALLOC(size) COREOBJ->AllocMem(size)
	#define CZFREE(ptr) COREOBJ->FreeMem(ptr)
	#define CZNATIVEALLOC(size) ::cz::NativeAllocMem(size)
	#define CZNATIVEFREE(ptr) ::cz::FreeNativeMem(ptr)
	#define CZNEW(CZOBJECTTYPE) new(COREOBJ) CZOBJECTTYPE
	#define CZDELETE(CZOBJECT) delete CZOBJECT
#endif // CZ_DEBUG



//
// Error callback macros
//
#if CZ_PLAYER_ERRORCALLBACK_ENABLED
	#define CZERRORCALLBACK(err) COREOBJ->OnError(err)
#else
	#define CZERRORCALLBACK(err) ((void)0)
#endif


//
// Error macros
//
#if defined(_MSC_VER) && (_MSC_VER < 1300)
	// VC6 doesn't support __FUNCTION__ macro
	#define CZERROR(err) \
		{ \
			CZERRORCALLBACK(err); \
			CZLOG("ERROR %d at %s:%d (%s)\n",err, __FILE__, __LINE__, COREOBJ->GetErrorMsg(err)); \
			return err; \
		}
			
	#define CZERROR_RETNULL(err) \
		{ \
			CZERRORCALLBACK(err); \
			CZLOG("ERROR %d at %s:%d (%s)\n",err, __FILE__, __LINE__, COREOBJ->GetErrorMsg(err)); \
			return NULL; \
		}
			
#else
	#define CZERROR(err) \
		{ \
			CZERRORCALLBACK(err); \
			CZLOG(LOG_ERROR, "ERROR %d at %s:%s:%d (%s)\n",err, __FILE__, __FUNCTION__, __LINE__, COREOBJ->GetErrorMsg(err)); \
			return err; \
		}
	#define CZERROR_RETNULL(err) \
		{ \
			CZERRORCALLBACK(err); \
			CZLOG(LOG_ERROR, "ERROR %d at %s:%s:%d (%s)\n",err, __FILE__, __FUNCTION__, __LINE__, COREOBJ->GetErrorMsg(err)); \
			return NULL; \
		}
#endif

//
// Assert macros
//
#if CZ_DEBUG
	#define CZASSERT(expr) { bool exp_res=(expr); if(!(exp_res)) CZLOG(LOG_ERROR, "ASSERT failed at %s:%d (%s)\n",__FILE__, __LINE__); assert(exp_res);  }
	// Difference between this and assert is this always evaluates the expression
	#define CZCHECK(expr) CZASSERT(expr)
#else
	#define CZASSERT(expr) ((void)0)
	// Difference between this and assert is this always evaluates the expression
	#define CZCHECK(expr) expr
#endif

//! This template allows for compile-time asserts
/*! I took the idea from the book "Modern C++ Design"
* Use the CTASSERT macro, but the expression used must be a compile-time constant
*
* Example:
*    CTASSERT(sizeof(SomeClass)==16);
*
* If the size of the the class "SomeClass" is not 16, then the code won't compile,
* and we know there is something wrong (This behavior IS the compile time assert).
* The code fails to compile if the expression inside CTASSERT is false,
* because the templated struct CTAssert is only defined for <true>
*/
template<bool> struct CTAssert;
template<> struct CTAssert<true> { };
#define CTASSERT(x) CTAssert<x>()


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
