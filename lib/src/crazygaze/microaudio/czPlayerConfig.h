/********************************************************************
	CrazyGaze (http://www.crazygaze.com)
	Author : Rui Figueira
	Email  : rui@crazygaze.com
	
	purpose:
	Conigures a czplayer build
*********************************************************************/

#ifndef _czPlayerConfig_h_
#define _czPlayerConfig_h_

namespace cz
{

//
// Flags used to configure the build, so you can decrease the codesize by cutting down on features
//
// CZ_PLAYER_LOG_ENABLED 0|1
//		Enables logging in general. If set to 0, czPlayer::Log isn't called at all



// Including to use custom tweaks if any
#include "config/czPlayerConfigCustom.h"

//
// Detect and/or fix inconsistencies with the configuration, or assume
// defaults where no custom value specified
// 

#ifdef _DOXYGEN
	#define CZ_PLAYER_ERRORCALLBACK_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to cause a call to "OnError" every time an error occurs*/
#else
	#ifndef CZ_PLAYER_ERRORCALLBACK_ENABLED
		#if (CZ_PLATFORM==CZ_PLATFORM_WIN32)
			#define CZ_PLAYER_ERRORCALLBACK_ENABLED 1
		#else
			#define CZ_PLAYER_ERRORCALLBACK_ENABLED 0
		#endif
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_EXTRAFUNCTIONS_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to compile rarely used functions */
#else
	#ifndef CZ_PLAYER_EXTRAFUNCTIONS_ENABLED
		#if (CZ_PLATFORM==CZ_PLATFORM_WIN32)
			#define CZ_PLAYER_EXTRAFUNCTIONS_ENABLED 1
		#else
			#define CZ_PLAYER_EXTRAFUNCTIONS_ENABLED 0
		#endif
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_WAV_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for WAV files */
#else
	#ifndef CZ_PLAYER_WAV_ENABLED
		#define CZ_PLAYER_WAV_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_MOD_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for MOD files */
#else
	#ifndef CZ_PLAYER_MOD_ENABLED
		#define CZ_PLAYER_MOD_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_IT_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for IT (Impulse tracker) files */
#else
	#ifndef CZ_PLAYER_IT_ENABLED
		#define CZ_PLAYER_IT_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_OGG_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable support for Ogg*/
#else
	#ifndef CZ_PLAYER_OGG_ENABLED
		#define CZ_PLAYER_OGG_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_DISKFILE_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable loading from files. If disabled, it will only compile support for loading through memory buffers*/
#else
	#ifndef CZ_PLAYER_DISKFILE_ENABLED
		#define CZ_PLAYER_DISKFILE_ENABLED 1
	#endif
#endif

#ifdef _DOXYGEN
	#define CZ_PLAYER_LOG_ENABLED 1 /**< @hideinitializer Set this to 0 or 1 to disable/enable logging*/
#else
	#ifndef CZ_PLAYER_LOG_ENABLED
		#if CZ_USER_BUILD
			#define CZ_PLAYER_LOG_ENABLED 0
		#else
			#define CZ_PLAYER_LOG_ENABLED 1
		#endif
	#endif
#endif



} // namespace cz

#endif // _czPlayerConfig_h_