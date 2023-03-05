#pragma once
#include <crazygaze/microaudio/Core.h>
#include <crazygaze/microaudio/Output.h>
#include <crazygaze/microaudio/AudioSource.h>
#include "../media/birds_48000.h"

using namespace cz::microaudio;
#define MySerial Serial1 

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
SimpleAudioSource gSrc(reinterpret_cast<const int16_t*>(birds_48000_raw), birds_48000_raw_numFrames);

void setup()
{
	MySerial.begin(115200);
	while (!MySerial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	MySerial.println("Starting");
	gOutput.begin(gSrc, nullptr);
	MySerial.println("Done");

	MemoryTracker::log();
}

void loop()
{
}
