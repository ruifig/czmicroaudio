
/*===============================================================================================
CrazyGaze (http://www.crazygaze.com)
Author : Rui Figueira
Email  : rui@crazygaze.com

This example shows the basic use of czPlayer, in its simplest form, without any callbacks.
To make thing even more clean, it doesn't even checks for errors.

===============================================================================================*/

#include <crazygaze/microaudio/Output.h>
#include <crazygaze/microaudio/AudioSource.h>
#include <crazygaze/microaudio/Core.h>
#include "../../../../media/birds_48000.h"

#include <Arduino.h>
#include <Windows.h>
#include <conio.h>


using namespace cz::microaudio;

struct MyCore : public Core
{

	// These are for the Logger interface
	virtual void onError(Error errorCode) override
	{
		Serial.print("ERROR: "); Serial.print(static_cast<int>(errorCode)); Serial.print(":"); Serial.println(getErrorMsg(errorCode));
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

		Serial.print(logLevelStr); Serial.print(":"); Serial.println(str);
	}
};

MyCore gCore;


#if 0

void setup()
{
#if 0
	while(!kbhit())
	{
	}
#endif

}

void loop()
{
	// Initialize player with default parameters
	AudioPlayer *player = AudioPlayer::Create();

	// Load sounds
	HSOUNDDATA hAmbientLoopData = player->LoadWAV("../media/554_bebeto_Ambient_loop_mono_11025.wav");
	HSOUNDDATA hWaterDropData = player->LoadWAV("../media/30341_junggle_waterdrop24_44100.wav");
	HMODULEDATA hTestMOD = player->LoadModule("../media/educationally_valuable.mod");
	HMODULEDATA hTestIT = player->LoadModule("../media/TESTSONG.it");
	
	// Play the looped sounds. You could have saved the return value to later control the playing sound
	//player->Play(hAmbientLoopData, cz::microaudio::AUDIO_MASTERVOL_MAX, -1, cz::microaudio::SOUND_LOOP_OFF);

	printf("1   - Play water drop (More presses will play the sound repeatedly)\n");
	printf("2   - Play MOD module\n");
	printf("3   - Play IT module\n");
	printf("ESC - Exit.\n");

	MemoryTracker::log();
	char key=0;
	do 
	{
		Sleep(10);
		if (kbhit())
		{
			key=getch();
			// We're using the water drop as "play and forget", so we don't need to keep track of the returned handlers
			if (key=='1')
				player->Play(hWaterDropData);
			if (key=='2')
				player->PlayModule(hTestMOD, 128);
			if (key=='3')
			{
				player->PlayModule(hTestIT, 128);
				MemoryTracker::log();
			}
		}
	} while(key!=27);

	// Released the loaded sound data
	player->FreeStaticSound(hAmbientLoopData);
	player->FreeStaticSound(hWaterDropData);
	player->FreeModuleSound(hTestMOD);
	player->FreeModuleSound(hTestIT);

	// Destroy in inverse order
	player->Destroy();
}

#else


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

constexpr size_t birds_48000_raw_numFrames = sizeof(birds_48000_raw) / sizeof(birds_48000_raw[0]) / 2;

SimpleAudioSource src(reinterpret_cast<const int16_t*>(birds_48000_raw), birds_48000_raw_numFrames);
DefaultOutput output;

void setup()
{
#if 0
	while(!kbhit())
	{
	}
#endif

	output.begin(src, nullptr);
}

void loop()
{
}

#endif
