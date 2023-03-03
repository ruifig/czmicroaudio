//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/Core.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <Arduino.h>

namespace cz::microaudio
{

const char* const g_czErrors[-(static_cast<int>(Error::Unknown)) + 1]={
	"Success",

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


const char *GetErrorMsg(Error error)
{
	int err = static_cast<int>(error);
	if ((err > 0) || (err < static_cast<int>(Error::Unknown)))
	{
		return "**INVALID ERROR CODE**";
	}
	else
	{
		return g_czErrors[-err];
	}
}

//////////////////////////////////////////////////////////////////////////////
// Core
//////////////////////////////////////////////////////////////////////////////

Core::Core()
{
	assert(ms_instance == nullptr);
	ms_instance = this;
}

Core::~Core()
{
	ms_instance = nullptr;
}

Core* Core::get()
{
	return ms_instance;
}

void Core::onError(Error error)
{
}

#if CZMICROAUDIO_LOG_ENABLED
void Core::onLog(LogLevel level, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char buf[256];
	vsprintf(buf,fmt, args);
	va_end(args);

	onLogSimple(level, buf);
}

void Core::onLogSimple(LogLevel, const char* str)
{
	Serial.println(str);
}

#endif

} // namespace cz
