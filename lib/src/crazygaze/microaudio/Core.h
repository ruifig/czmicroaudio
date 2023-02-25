//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#ifndef _CZ_CORE_H_
#define _CZ_CORE_H_

#include <crazygaze/microaudio/Player.h>
#include <crazygaze/microaudio/MemoryProvider.h>
#include <crazygaze/microaudio/Logger.h>

namespace cz
{
	
	// forward declarations
	class MemoryProviders;
	class AudioLogger;
	class Core;

	//! Returns the description of an error.
	const char* GetErrorMsg(int errCode);
	//! Returns a string with the build date and time of the library
	const char* GetBuildTimeStamp();
	
	/*!
	 * \brief Use internally for some tests
	 * \warning Don't use this yourself, since it's probably not working properly.
	 * \todo This needs to be fixed for some platforms.
	 */

	int32_t GetMicroseconds();

	/*!
	 * \brief Core creation configuration
	 *
	 * You customize some parts of the library, like providing your own memory
	 * manager, or logger.
	 * 
	 */
	struct CoreConfig
	{
		CoreConfig() : memoryProvider(0), logger(0) {};

		/*!
		 * \brief Your own memory manager.
		 * Set to NULL to use the default one
		 */
		MemoryProvider *memoryProvider;

		/*!
		 * \brief Your own logger
		 * Set to NULL to ignore logs
		 */
		AudioLogger *logger;
	};

	
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


		static Core* Create(CoreConfig *cfg=0);
		void Destroy();

		void* AllocMem(size_t size, const char *file=NULL, int line=0);
		void FreeMem(void *ptr, const char *file=NULL, int line=0);
		const char *GetErrorMsg(int err);
		void OnError(int errorType);
#if CZ_PLAYER_LOG_ENABLED
		void OnLog(LogLevel level, const char *fmt, ...);
#endif

	private:
		MemoryProvider* m_memoryProvider;
		AudioLogger* m_logger;
		void InitGlobalPointers();


	private:

	};




}

#endif

