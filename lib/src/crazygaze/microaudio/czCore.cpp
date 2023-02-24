//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czCore.h"
#include "czStandardC.h"
#include "czPlayerPrivateDefs.h"

#include "globals/czITModuleTables.h"
#include "globals/czMODModuleTables.h"

namespace cz
{

const char* const g_czErrors[-ERR_UNKNOWN]={
	"Out of memory",
	"Invalid parameter",
	"Resource, hardware, or option not available or not detected",
	"Can't execute",

	"Error opening/creating file",
	"Error closing file",
	"IO error in file",

	"Invalid file format",
	"Invalid data",

	"Error calling OS API",

	"Unknown error"
};


const char *GetErrorMsg(int err)
{
	if((err>-1)||(err<ERR_UNKNOWN)) return NULL;
	return (char*) g_czErrors[-err-1];
}

const char* GetBuildTimeStamp()
{
	return __DATE__" " __TIME__;
}

void* AllocNativeMem(size_t size)
{
	return malloc(size);
}
void FreeNativeMem(void *ptr)
{
	free(ptr);
}

void* Core::AllocMem(size_t size, const char* CZUNUSED(file), int CZUNUSED(line))
{
	if (m_memoryProvider)
		return m_memoryProvider->AllocMem(size);
	else
		return AllocNativeMem(size);
}
void Core::FreeMem(void *ptr, const char* CZUNUSED(file), int CZUNUSED(line))
{
	if (m_memoryProvider)
		m_memoryProvider->FreeMem(ptr);
	else
		FreeNativeMem(ptr);
}

void Core::OnError(int errorType)
{
	if (m_logger)
		m_logger->OnError(errorType);
}

const char *Core::GetErrorMsg(int err)
{
	if((err>-1)||(err<ERR_UNKNOWN)) return NULL;
	return (char*) errorStrings[-err-1];
}



#if CZ_PLAYER_LOG_ENABLED

void Core::OnLog(LogLevel level, const char *fmt, ...)
{
	if (!m_logger)
		return;

	va_list args;
	va_start(args, fmt);
	char buf[256];
	vsprintf(buf,fmt, args);
	va_end(args);

	m_logger->OnLog(level, buf);
}
#endif

void Core::InitGlobalPointers()
{
	//	char **g = g_czErrors;

	errorStrings = g_czErrors;

	// Tables for IT support
	IT_pantable = audio::g_IT_pantable;
	IT_SlideTable = audio::g_IT_SlideTable;
	IT_FineSineData = audio::g_IT_FineSineData;
	IT_FineRampDownData = audio::g_IT_FineRampDownData;
	IT_FineSquareWave = audio::g_IT_FineSquareWave;
	IT_PitchTable = (const uint32_t*)audio::g_IT_PTable;
	IT_FineLinearSlideUpTable = (const uint32_t*) audio::g_IT_FLSUTable;
	IT_LinearSlideUpTable = (const uint32_t*) audio::g_IT_LSUTable;
	IT_FineLinearSlideDownTable = audio::g_IT_FLSDTable;
	IT_LinearSlideDownTable = audio::g_IT_LSDTable;

	// Tables for MOD support
	MOD_periods = audio::g_MOD_periods;
	MOD_waveFormsPtrs[0]= audio::g_MOD_sineWave;
	MOD_waveFormsPtrs[1]= audio::g_MOD_rampdownWave;
	MOD_waveFormsPtrs[2]= audio::g_MOD_squareWave;
	MOD_waveFormsPtrs[3]= audio::g_MOD_randomWave;

}


Core* Core::Create(CoreConfig *cfg)
{
	// Allocate the structured with the czPlayer core data
	// We need to check if a memory provider was specified. If not, we need to take care of the struct memory ourselves
	Core *core=NULL;
	if (cfg && cfg->memoryProvider)
		core = (Core*) cfg->memoryProvider->AllocMem(sizeof(Core));
	else
		core = (Core*) AllocNativeMem(sizeof(Core));

	if (!core)
		return NULL;

	memset(core, 0, sizeof(Core));

	core->InitGlobalPointers();

	if (cfg)
	{
		core->m_memoryProvider = cfg->memoryProvider;
		core->m_logger = cfg->logger;
	}

	return core;
}

void Core::Destroy()
{
	if (m_memoryProvider)
		m_memoryProvider->FreeMem(this);
	else
		FreeNativeMem(this);
}


} // namespace cz
