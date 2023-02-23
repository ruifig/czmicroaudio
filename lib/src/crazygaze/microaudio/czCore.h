//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#ifndef _CZ_CORE_H_
#define _CZ_CORE_H_

#include "czPlayer.h"
#include "czMemoryProvider.h"
#include "czLogger.h"

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

	s32 GetMicroseconds();

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

	//
	// :TODO: - Internal stuff. Don't mess with it. I'll have to get a better design for this
	// This is used to access global data needed in the mixer thread, because PALM
	// can't access global pointers in the sound callback
	//
	const char* const * errorStrings;
	// data for IT support
	const u8* IT_pantable;
	const u8* IT_SlideTable;
	const s8* IT_FineSineData;
	const s8* IT_FineRampDownData;
	const s8* IT_FineSquareWave;
	const u32* IT_PitchTable;
	const u32* IT_FineLinearSlideUpTable;
	const u32* IT_LinearSlideUpTable;
	const u16* IT_FineLinearSlideDownTable;
	const u16* IT_LinearSlideDownTable;
	// data for MOD support
	const s16* MOD_periods;
	const s8* MOD_waveFormsPtrs[4];

	private:
		MemoryProvider* m_memoryProvider;
		AudioLogger* m_logger;
		void InitGlobalPointers();


	private:

	};




}

#endif

