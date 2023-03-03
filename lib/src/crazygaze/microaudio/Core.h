//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#pragma once

#include <crazygaze/microaudio/Player.h>

namespace cz::microaudio
{
	
	//! Returns the description of an error.
	const char* GetErrorMsg(Error error);


	/*!
	 * \brief Provides core functionality needed across the library
	 *
	 * This is the first class you need to use before any other.
	 * Use the static #Create method to create an instance, and the #Destroy method to destroy.
	 * This should be the first object to create, and the last one to destroy.
	 */
	class Core
	{
	public:

		Core();
		virtual ~Core();

		/**
		 * Returns the singleton
		 */
		static Core* get();

		virtual void onError(Error error);
#if CZMICROAUDIO_LOG_ENABLED
		virtual void onLog(LogLevel level, const char *fmt, ...);
		virtual void onLogSimple(LogLevel, const char *str);
#endif

	private:
		inline static Core* ms_instance = nullptr;
	};

}


