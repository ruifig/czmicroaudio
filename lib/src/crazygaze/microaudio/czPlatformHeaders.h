#pragma once

#include <stdint.h>

#if defined(_WIN32)
	#define CZ_PLATFORM_WIN32 1
	#define CZ_PLATFORM_ARDUINO 0
#elif defined(ARDUINO)
	#define CZ_PLATFORM_WIN32 0
	#define CZ_PLATFORM_ARDUINO 1
#elif
	#error Unknown platform
#endif

#if defined(DEBUG) || defined(_DEBUG)
	#define CZ_DEBUG 1
#else
	
	#if !defined(NDEBUG)
		#error No _DEBUG/DEBUG or NDEBUG defined
	#endif

	#define CZ_DEBUG 0
#endif

#if CZ_PLATFORM_WIN32
	// System includes
#elif CZ_PLATFORM_ARDUINO
	// System includes
#endif


