//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#pragma once

#include <crazygaze/microaudio/Config.h>
#include <crazygaze/microaudio/Mixer.h>
#include <crazygaze/microaudio/AudioPlayer.h>

namespace cz::microaudio
{

// forward declarations
class StaticSound;
class Module;
class StreamSound;
class File;

enum SoundType
{
	STATICSOUND = 1,
	STREAMSOUND = 2,
	MODULESOUND = 3
};

typedef struct
{
	union
	{
		StaticSound *staticSound;
		StreamSound *streamSound;
		Module *moduleSound;
	};
	HSOUND handle;
	int mixerChannel; // -1 if N/A, or >=0 if it uses a single channel
	SoundType soundType;
	bool isStream;
	bool feedByBlocks;
	//TTime msAtStreamEnd; // workaround to avoid stopping streams suddenly when they reach the end
	int32_t circularFlag; // Flag used to decide when to feed the new data.

	bool paused; // Tells if the sound is paused

	int bpmMixTodo; // Used to control the calls to DoTick() method when playing a Module;
} SOUND_ST;



//! Base class for sound output
/*!
* This class provides the interface to make things happens. Output sound.
*
* You can play/stop sounds, change volume, frequency, panning, etc.
* When you want to play a song or sound, you use the respective \link czStaticSound \endlink, or the \link czModule \endlink,
* and you'll get a handler of type \link ::HSOUND \endlink, which you can use to control the playback of the sound.
* 
* \sa \link Play \endlink
* \sa \link PlayModule \endlink
*/
class SoundOutput : public ::cz::microaudio::AudioPlayer
{
public:	
	SoundOutput();
	virtual ~SoundOutput();

	//
	//
	// czPlayerInterface methods
	//
	//
	virtual void Destroy();

	virtual int Update();
#if CZMICROAUDIO_DISKFILE_ENABLED
	virtual HMODULEDATA LoadModule(const char* filename);
#endif
	virtual HMODULEDATA LoadModule(const void* data, int dataSize);
#if CZMICROAUDIO_DISKFILE_ENABLED
	virtual HSOUNDDATA LoadWAV(const char* filename);
#endif
	virtual HSOUNDDATA LoadWAV(const void *data, int dataSize);
	virtual int FreeModuleSound(HMODULEDATA handle);
	virtual int FreeStaticSound(HSOUNDDATA handle);

#if CZMICROAUDIO_OGG_ENABLED
#if CZMICROAUDIO_DISKFILE_ENABLED
	virtual HMODULEDATA LoadStream(const char* filename);
#endif
	virtual HMODULEDATA LoadStream(const void* data, int dataSize);
	virtual int FreeStream(HSOUNDDATA handle);
#endif

	virtual void SetInterpolationMode(AudioInterpolationMode quality);
	virtual AudioInterpolationMode GetInterpolationMode();
	virtual void SetVolumeRamping(int numFrames);
	virtual int GetVolumeRamping();
	virtual void SetStopRamping(int numFrames);
	virtual int GetStopRamping();
	virtual int GetOutputFrequency(void);
	virtual int GetMaxChannels(void);
	virtual int GetChannelsPlaying(void);
	virtual int GetOutputChannels(void);
	virtual int GetOutputBits(void);
	virtual HSOUND Play(HSOUNDDATA sndDataHandle, int vol=-1, int pan=-1, LoopMode loopMode= SOUND_LOOP_OFF, int loopStart=0, int loopLength=0);
	virtual HSOUND PlayModule(HMODULEDATA modDataHandle, int masterVol=AUDIO_VOL_MAX/2, bool loop=false, int firstOrder=0, int lastOrder=-1);
	virtual HSOUND PlayStream(HSTREAMDATA sndDataHandle, int vol=AUDIO_VOL_MAX/2, bool loop=false);
	virtual int Stop(HSOUND sndHandle);
	virtual int StopAll(void);
	virtual int SetVolume(HSOUND sndHandle, uint8_t vol);
	virtual int SetFrequency(HSOUND sndHandle, int freq);
	virtual int SetPanning(HSOUND sndHandle, uint8_t pan);
	virtual int Pause(HSOUND sndHandle);
	virtual int Resume(HSOUND sndHandle);
	virtual AudioPlayerListener* SetListener(AudioPlayerListener *listener);
	virtual int SetSFXMasterVolume(int vol);
	virtual int GetSFXMasterVolume(void);

	//virtual void* GetParent();

protected:
	friend ::cz::microaudio::AudioPlayer* ::cz::microaudio::AudioPlayer::Create(AudioPlayerConfig *cfg/* =NULL */);
	int FeedData(void *ptr, int numFrames);
	void UpdateStatus(void);
	virtual int Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq );
	int InitSoftwareMixerOutput(int maxActiveSounds, int mixSize, bool stereo, bool bits16, int freq );

	virtual void LockMixer() = 0;
	virtual void UnlockMixer() = 0;
	
	Mixer m_mixer;
	AudioPlayerListener *m_listener;

private:

//	czMixer* GetMixer(void) { return &m_mixer; };

	void UpdateStreamSound(SOUND_ST *st);
	void UpdateModuleSound(SOUND_ST *st);
	void UpdateStaticSound(SOUND_ST *st);

	Module* LoadModule(File *in);
	StaticSound* LoadWAV(File *in);
#if CZMICROAUDIO_OGG_ENABLED
	StreamSound* LoadStream(File *in);
#endif


	int m_maxActiveSounds;
	int m_sfxMasterVolume;
	SOUND_ST *m_sounds;
	SOUND_ST *GetEmptySlot(void);
	SOUND_ST *GetSlot(HSOUND sndHandle);
	uint32_t m_lastHandle;

#if CZMICROAUDIO_OGG_ENABLED
	//WorkerThread m_iothread;
#endif

public:

};

} // namespace cz::microaudio

