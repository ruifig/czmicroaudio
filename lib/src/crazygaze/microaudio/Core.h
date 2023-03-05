#pragma once

#include <crazygaze/microaudio/Defs.h>

namespace cz::microaudio
{
	//! Returns the description of an error.
	const char* getErrorMsg(Error error);

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

#if CZMICROAUDIO_ERRORCALLBACK_ENABLED
		virtual void onError(Error error);
#endif

#if CZMICROAUDIO_LOG_ENABLED
		virtual void onLog(LogLevel level, const char *fmt, ...);
		virtual void onLogSimple(LogLevel, const char *str);
#endif

	private:
		inline static Core* ms_instance = nullptr;
	};

}


