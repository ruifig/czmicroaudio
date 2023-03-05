#pragma once

//
// Flags used to configure the build, so you can decrease the code size by cutting down on features
//

#if defined(_WIN32)
	#define CZMICROAUDIO_PLATFORM_WINDOWS 1
	#define CZMICROAUDIO_PLATFORM_ARDUINO 0
#elif defined(ARDUINO)
	#define CZMICROAUDIO_PLATFORM_WINDOWS 0
	#define CZMICROAUDIO_PLATFORM_ARDUINO 1
#elif
	#error Unknown platform
#endif

#if defined(DEBUG) || defined(_DEBUG)
	#define CZMICROAUDIO_DEBUG 1
#else
	#if !defined(NDEBUG)
		#error No _DEBUG/DEBUG or NDEBUG defined
	#endif
	#define CZMICROAUDIO_DEBUG 0
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#if CZMICROAUDIO_PLATFORM_WINDOWS
	// System includes
#elif CZMICROAUDIO_PLATFORM_ARDUINO
	// System includes
#endif

//
// Detect and/or fix inconsistencies with the configuration, or assume
// defaults where no custom value specified
// 

#ifndef CZMICROAUDIO_ERRORCALLBACK_ENABLED
	#if CZMICROAUDIO_DEBUG
		#define CZMICROAUDIO_ERRORCALLBACK_ENABLED 1
	#else
		#define CZMICROAUDIO_ERRORCALLBACK_ENABLED 0
	#endif
#endif

#ifndef CZMICROAUDIO_WAV_ENABLED
	#define CZMICROAUDIO_WAV_ENABLED 1
#endif

#ifndef CZMICROAUDIO_MOD_ENABLED
	#define CZMICROAUDIO_MOD_ENABLED 1
#endif

#ifndef CZMICROAUDIO_IT_ENABLED
	#define CZMICROAUDIO_IT_ENABLED 1
#endif

#ifndef CZMICROAUDIO_OGG_ENABLED
	#define CZMICROAUDIO_OGG_ENABLED 0
#endif

#ifndef CZMICROAUDIO_DISKFILE_ENABLED
	#define CZMICROAUDIO_DISKFILE_ENABLED 1
#endif

#ifndef CZMICROAUDIO_LOG_ENABLED
	#if CZMICROAUDIO_DEBUG
		#define CZMICROAUDIO_LOG_ENABLED 1
	#else
		#define CZMICROAUDIO_LOG_ENABLED 0
	#endif
#endif

#ifndef CZMICROAUDIO_MEMTRACKER_ENABLED
	#if CZMICROAUDIO_DEBUG
		#define CZMICROAUDIO_MEMTRACKER_ENABLED 1
	#else
		#define CZMICROAUDIO_MEMTRACKER_ENABLED 0
	#endif
#endif

