#include <crazygaze/microaudio/Core.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <Arduino.h>

namespace cz::microaudio
{

const char *getErrorMsg(Error error)
{
	static const char* const errors[static_cast<int>(Error::Unknown)+1]={
		"Success",
		"OutOfMemory",
		"InvalidParameter",
		"NotAvailable",
		"CantRun",
		"CantOpen",
		"CantClose",
		"IOError",
		"WrongFormat",
		"InvalidData",
		"BadAPICall",
		"Unknown"
	};

	int err = static_cast<int>(error);
	CZMICROAUDIO_ASSERT(err>=0 && err<=static_cast<int>(Error::Unknown));
	return errors[err];
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
	assert(ms_instance);
	return ms_instance;
}

#if CZMICROAUDIO_ERRORCALLBACK_ENABLED
void Core::onError(Error error)
{
}
#endif

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
