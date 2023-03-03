
/*===============================================================================================
CrazyGaze (http://www.crazygaze.com)
Author : Rui Figueira
Email  : rui@crazygaze.com

This example shows the basic use of czPlayer, in its simplest form, without any callbacks.
To make thing even more clean, it doesn't even checks for errors.

===============================================================================================*/

#include <crazygaze/microaudio/All.h>

#include <Arduino.h>
#include <Windows.h>
#include <conio.h>


using namespace cz::microaudio;

struct MyCore : public Core
{

	// These are for the Logger interface
	virtual void onError(Error errorCode) override
	{
		Serial.print("ERROR: "); Serial.print(errorCode); Serial.print(":"); Serial.println(GetErrorMsg(errorCode));
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
	cz::microaudio::AudioPlayer *player = cz::microaudio::AudioPlayer::Create();

	// Load sounds
	cz::microaudio::HSOUNDDATA hAmbientLoopData = player->LoadWAV("../media/554_bebeto_Ambient_loop_mono_11025.wav");
	cz::microaudio::HSOUNDDATA hWaterDropData = player->LoadWAV("../media/30341_junggle_waterdrop24_44100.wav");
	cz::microaudio::HMODULEDATA hTestMOD = player->LoadModule("../media/educationally_valuable.mod");
	cz::microaudio::HMODULEDATA hTestIT = player->LoadModule("../media/TESTSONG.it");
	
	// Play the looped sounds. You could have saved the return value to later control the playing sound
	//player->Play(hAmbientLoopData, cz::microaudio::AUDIO_MASTERVOL_MAX, -1, cz::microaudio::SOUND_LOOP_OFF);

	printf("1   - Play water drop (More presses will play the sound repeatedly)\n");
	printf("2   - Play MOD module\n");
	printf("3   - Play IT module\n");
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
			if (key=='2')
				player->PlayModule(hTestMOD, 128);
			if (key=='3')
				player->PlayModule(hTestIT, 128);
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

