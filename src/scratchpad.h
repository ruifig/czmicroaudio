#pragma once
#include <crazygaze/microaudio/Core.h>
#include <crazygaze/microaudio/Output.h>
#include <crazygaze/microaudio/AudioSource.h>
#include "../media/birds_48000.h"
#include "SerialStringReader.h"

#if CZMICROAUDIO_PLATFORM_ARDUINO
	#define USE_WAVESHARE_PICO_AUDIO 1
#else
	#define USE_WAVESHARE_PICO_AUDIO 0
#endif

#if USE_WAVESHARE_PICO_AUDIO
	// minimal example to setup MCLK clock with PIO, used that in combination with I2S library
	#include "MCLK/MCLK.h"
	// changing clock
	#include <pico/stdlib.h>

	struct FakeMCLK
	{
		void setPin(int) {}
		bool begin(int, int) { return true;}
	};
	MCLK mclk;
#endif

using namespace cz::microaudio;
#define MySerial Serial1 

cz::SerialStringReader<> gSerialStringReader;

struct MyCore : public Core
{

	// These are for the Logger interface
	virtual void onError(Error errorCode) override
	{
		MySerial.print("ERROR: "); MySerial.print(static_cast<int>(errorCode)); MySerial.print(":"); MySerial.println(getErrorMsg(errorCode));
	}

	virtual void onLogSimple(LogLevel level, const char *str) override
	{
		const char* logLevelStr;
		if (level == LogLevel::Fatal)
			logLevelStr = "FATAL   : ";
		else if (level == LogLevel::Error)
			logLevelStr = "ERROR   : ";
		else if (level == LogLevel::Warning)
			logLevelStr = "WARNING : ";
		else if (level == LogLevel::Log)
			logLevelStr = "INFO    : ";
		else
			logLevelStr = "LOG     : ";

		MySerial.print(logLevelStr); MySerial.print(":"); MySerial.println(str);
	}
};

class SimpleAudioSource : public AudioSource
{
public:
	SimpleAudioSource(const int16_t* data, uint32_t numFrames)
		: m_data(data)
		, m_numFrames(numFrames)
	{
		m_end = data + numFrames*2;
		m_pos = m_data;
	}

	uint32_t get(void* ptr, uint32_t numFrames) override
	{
		int16_t* p = reinterpret_cast<int16_t*>(ptr);
		uint32_t todo = numFrames;
		while(todo--)
		{
			// left
			*p = *m_pos;
			p++;
			m_pos++;

			// right
			*p = *m_pos;
			p++;
			m_pos++;

			if (m_pos >= m_end)
				m_pos = m_data;
		}

		return numFrames;
	}

private:

	const int16_t* m_data;
	const int16_t* m_end;
	const int16_t* m_pos;
	uint32_t m_numFrames;
};

MyCore gCore;
DefaultOutput gOutput;
constexpr size_t birds_48000_raw_numFrames = sizeof(birds_48000_raw) / sizeof(birds_48000_raw[0]) / 2;

#if USE_WAVESHARE_PICO_AUDIO
#endif


SimpleAudioSource gSrc(reinterpret_cast<const int16_t*>(birds_48000_raw), birds_48000_raw_numFrames);

void setup()
{
	MySerial.begin(115200);
	gSerialStringReader.begin(MySerial);

	while (!MySerial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	MySerial.println("Starting");

	OutputConfig cfg;
#if USE_WAVESHARE_PICO_AUDIO
	DefaultOutput::Options opts;
	opts.DATAPin = 22;
	mclk.setPin(26);
	opts.baseClockPin = 27;
	opts.swapClocks = true;
	cfg.sampleRate = 48000;
	cfg.platformOptions = &opts;
	constexpr int mclkMultiplier = 192;

	// Set CPU speed for a good ratio with Multipler*fs -- flag to block if requested speed cannot be achieved
	// Don't really understand these multipliers well yet, but this seems to work for me
	MySerial.print("Setting sysclock...");
	set_sys_clock_khz(cfg.sampleRate*4, true);
	MySerial.println("done.");

	MySerial.print("Starting mclk...");
	if (mclk.begin(cfg.sampleRate, mclkMultiplier))
	{
		MySerial.println("done.");
	}
	else
	{
		while (1)
		{
			MySerial.println("Failed to initialize MCLK!");
			delay(1000);
		}
	}
#endif

	gOutput.begin(gSrc, &cfg);
	MySerial.println("Done");

	//MemoryTracker::log();
}

void loop()
{
	if (gSerialStringReader.tryRead())
	{
		const char* cmd = gSerialStringReader.retrieve();
		MySerial.print("CMD: ");
		MySerial.println(cmd);
	}
}
