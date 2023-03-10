/********************************************************************
	CrazyGaze (http://www.crazygaze.com)
	Author : Rui Figueira
	Email  : rui@crazygaze.com
	
	purpose:
	Conigures a czplayer build
*********************************************************************/

#pragma once

//
// Flags used to configure the build, so you can decrease the codesize by cutting down on features
//
// CZMICROAUDIO_LOG_ENABLED 0|1
//		Enables logging in general. If set to 0, czPlayer::Log isn't called at all

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

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#if CZ_PLATFORM_WIN32
	// System includes
#elif CZ_PLATFORM_ARDUINO
	// System includes
#endif

// Including to use custom tweaks if any
#include <crazygaze/microaudio/config/CustomConfig.h>


//
// Detect and/or fix inconsistencies with the configuration, or assume
// defaults where no custom value specified
// 

#ifdef _DOXYGEN
	#define CZMICROAUDIO_ERRORCALLBACK_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to cause a call to "OnError" every time an error occurs*/
#else
	#ifndef CZMICROAUDIO_ERRORCALLBACK_ENABLED
		#if CZ_PLATFORM_WIN32
			#define CZMICROAUDIO_ERRORCALLBACK_ENABLED 1
		#else
			#define CZMICROAUDIO_ERRORCALLBACK_ENABLED 0
		#endif
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_EXTRAFUNCTIONS_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to compile rarely used functions */
#else
	#ifndef CZMICROAUDIO_EXTRAFUNCTIONS_ENABLED
		#if CZ_PLATFORM_WIN32
			#define CZMICROAUDIO_EXTRAFUNCTIONS_ENABLED 1
		#else
			#define CZMICROAUDIO_EXTRAFUNCTIONS_ENABLED 0
		#endif
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_WAV_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for WAV files */
#else
	#ifndef CZMICROAUDIO_WAV_ENABLED
		#define CZMICROAUDIO_WAV_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_MOD_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for MOD files */
#else
	#ifndef CZMICROAUDIO_MOD_ENABLED
		#define CZMICROAUDIO_MOD_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_IT_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for IT (Impulse tracker) files */
#else
	#ifndef CZMICROAUDIO_IT_ENABLED
		#define CZMICROAUDIO_IT_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_OGG_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for Ogg*/
#else
	#ifndef CZMICROAUDIO_OGG_ENABLED
		#define CZMICROAUDIO_OGG_ENABLED 0
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_DISKFILE_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable loading from files. If disabled, it will only compile support for loading through memory buffers*/
#else
	#ifndef CZMICROAUDIO_DISKFILE_ENABLED
		#define CZMICROAUDIO_DISKFILE_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZMICROAUDIO_LOG_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable logging*/
#else
	#ifndef CZMICROAUDIO_LOG_ENABLED
		#if CZ_DEBUG
			#define CZMICROAUDIO_LOG_ENABLED 1
		#else
			#define CZMICROAUDIO_LOG_ENABLED 0
		#endif
	#endif
#endif


#ifndef CZMICROAUDIO_MEMTRACKER_ENABLED
	#if CZ_DEBUG
		#define CZMICROAUDIO_MEMTRACKER_ENABLED 1
	#else
		#define CZMICROAUDIO_MEMTRACKER_ENABLED 0
	#endif
#endif

