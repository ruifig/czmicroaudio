
/*===============================================================================================
CrazyGaze (http://www.crazygaze.com)
Author : Rui Figueira
Email  : rui@crazygaze.com

This example shows the basic use of czPlayer, in its simplest form, without any callbacks.
To make thing even more clean, it doesn't even checks for errors.

===============================================================================================*/

#include <Arduino.h>

#include "crazygaze/microaudio/czAll.h"
#include "crazygaze/microaudio/czLogger.h"
#include <Windows.h>
#include <conio.h>


struct Framework : public cz::AudioLogger, public cz::MemoryProvider
{
	// These are for the Logger interface
	virtual void OnError(int errorCode) override
	{
		Serial.print("ERROR: "); Serial.print(errorCode); Serial.print(":"); Serial.println(cz::GetErrorMsg(errorCode));
	}

	virtual void OnLog(cz::LogLevel level, const char *str) override
	{
		const char* logLevelStr;
		if (level == cz::LOG_INFO)
			logLevelStr = "INFO    : ";
		else if (level == cz::LOG_WARNING)
			logLevelStr = "WARNING : ";
		else if (level == cz::LOG_ERROR)
			logLevelStr = "ERROR   : ";
		else
			logLevelStr = "LOG     : ";

		Serial.print(logLevelStr); Serial.print(":"); Serial.println(str);
	}

	// These are for the MemoryProvider interface
	virtual void *AllocMem(size_t size)
	{
		return malloc(size);
	}

	virtual void FreeMem(void *ptr)
	{
		free(ptr);
	}
};

void setup()
{
}

void loop()
{
	// Initialize core and get a player interface, with all the default values
	Framework framework;
	cz::CoreConfig cfg;
	cfg.memoryProvider = &framework;
	cfg.logger = &framework;
	cz::Core *core = cz::Core::Create(&cfg);
	cz::microaudio::AudioPlayer *player = cz::microaudio::AudioPlayer::Create(core);

	// Load sounds
	cz::microaudio::HSOUNDDATA hAmbientLoopData = player->LoadWAV("../media/554_bebeto_Ambient_loop_mono_11025.wav");
	cz::microaudio::HSOUNDDATA hWaterDropData = player->LoadWAV("../media/30341_junggle_waterdrop24_44100.wav");
	
	// Set ambient sound loop to all
	//player->SetSoundDefaultLoop(hAmbientLoopData, cz::microaudio::AUDIO_LOOP_NORMAL);
	// Change the default volume for the ambient loop to a lot lower, so we can hear the water drop more clearly
	//player->SetSoundDefaults(hAmbientLoopData,-1,cz::microaudio::AUDIO_VOL_MAX/4);

	cz::microaudio::HSOUND hAmbientLoop=0; // Used to keep control of the ambient sound

	// Play the looped sounds. You could have saved the return value to later control the playing sound
	player->Play(hAmbientLoopData, -1, -1, cz::microaudio::SOUND_LOOP_NORMAL);

	printf("1   - Play water drop (More presses will play the sound repeatedly)\n");
	printf("ESC - Exit.\n");
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
		}
	} while(key!=27);


	// Released the loaded sound data
	player->FreeStaticSound(hAmbientLoopData);
	player->FreeStaticSound(hWaterDropData);

	// Destroy in inverse order
	player->Destroy();
	core->Destroy();
}

