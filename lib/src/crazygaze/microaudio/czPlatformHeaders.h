#pragma once

#if defined(_WIN32)
	#define CZ_PLATFORM_WIN32 1
	#define CZ_PLATFORM_ARDUINO 0
#elif defined(ARDUINO)
	#define CZ_PLATFORM_WIN32 0
	#define CZ_PLATFORM_ARDUINO 1
#elif
	#error Unknown platform
#endif

#include <stdint.h>

#if CZ_PLATFORM_WIN32
	// System includes
#elif CZ_PLATFORM_ARDUINO
	// System includes
#endif

