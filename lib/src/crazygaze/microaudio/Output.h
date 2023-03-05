#pragma once

#include <crazygaze/microaudio/Config.h>

#if CZMICROAUDIO_PLATFORM_WINDOWS
	#include <crazygaze/microaudio/output/win32/Win32WaveOutOutput.h>
	namespace cz::microaudio
	{
		using DefaultOutput = Win32WaveOutOutput;
	}
#elif CZMICROAUDIO_PLATFORM_ARDUINO
	#include <crazygaze/microaudio/output/arduino/ArduinoI2SOutput.h>
	namespace cz::microaudio
	{
		using DefaultOutput = ArduinoI2SOutput;
	}
#endif

