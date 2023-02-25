//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/SoundOutput.h>
#include <crazygaze/microaudio/StaticSound.h>
#include <crazygaze/microaudio/Module.h>
#include <crazygaze/microaudio/Stream.h>
#include <crazygaze/microaudio/MemFile.h>
#include <crazygaze/microaudio/WAVLoader.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>

#if CZ_PLATFORM_WIN32
	#include <crazygaze/microaudio/win32/Win32WaveOutOutput.h>
#elif CZ_PLATFORM_ARDUINO
	#include <crazygaze/microaudio/arduino/ArduinoI2SOutOutput.h>
#else
	#error Unknown platform
#endif

#if CZ_PLAYER_MOD_ENABLED
	#include <crazygaze/microaudio/MODModule.h>
#endif

#if CZ_PLAYER_IT_ENABLED
	#include <crazygaze/microaudio/ITModule.h>
#endif

#if CZ_PLAYER_DISKFILE_ENABLED
	#include <crazygaze/microaudio/DiskFile.h>
#endif

namespace cz
{
namespace microaudio
{


int AudioPlayer::GetDefaultConfig(AudioPlayerConfig *cfg)
{
	memset(cfg,0,sizeof(*cfg));

#if CZ_PLATFORM_WIN32
	cfg->driverType = AUDIO_DRIVER_WINMM;
	cfg->bufSizeMs=200;
	cfg->frequency = 48000;
	cfg->bits = 16;
	cfg->outputChannels=2;
	cfg->maxChannels=128;
	cfg->interpolationMode = AUDIO_INTERPOLATION_LINEAR;
#elif CZ_PLATFORM_ARDUINO
	cfg->driverType = AUDIO_DRIVER_ARDUINO_I2S;
	cfg->bufSizeMs=50;
	cfg->frequency = 16000;
	cfg->bits = 16;
	cfg->outputChannels=2;
	cfg->maxChannels=32;
	cfg->interpolationMode = AUDIO_INTERPOLATION_NONE;
#else
	#error No platform specific configuration
#endif

	return ERR_OK;
}



/**********************************************************/
/***************** czSoundOutput ******************/
/**********************************************************/

SoundOutput::SoundOutput(::cz::Core *parentObject) : ::cz::Object(parentObject), m_mixer(parentObject)
{
	PROFILE();

	m_sounds = NULL;
	m_lastHandle = 0;
	m_listener = NULL;
}

SoundOutput::~SoundOutput()
{
	PROFILE();

	if (m_sounds!=NULL){
		StopAll();
		CZFREE(m_sounds);
	}

#if CZ_PLAYER_OGG_ENABLED
	//m_iothread.Shutdown(true);
#endif

	m_mixer.Free();
}

int SoundOutput::Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq )
{
#if CZ_PLAYER_OGG_ENABLED
	/*
	CZLOG(LOG_INFO, "Create IO thread...\n");
	if (!m_iothread.Create(0, PRIORITY_NORMAL)!=ERR_OK)
	{
		CZERROR(ERR_BADAPICALL);
	}
	m_iothread.Start();
	CZLOG(LOG_INFO, "IO thread created\n");
	*/
#endif

	return ERR_OK;
}

int SoundOutput::InitSoftwareMixerOutput(int maxActiveSounds, int mixSize, bool stereo, bool bits16, int freq )
{
	PROFILE();

	m_maxActiveSounds = maxActiveSounds;
	m_sfxMasterVolume = AUDIO_MASTERVOL_DEFAULT;
	m_sounds = (SOUND_ST*) CZALLOC(sizeof(SOUND_ST)*m_maxActiveSounds);
	if (m_sounds==NULL) CZERROR(ERR_NOMEM);
	memset(m_sounds, 0, sizeof(SOUND_ST)*m_maxActiveSounds);

	int ret = m_mixer.Init(maxActiveSounds, mixSize, stereo, bits16, freq);
	if (ret!=ERR_OK) CZERROR(ret);

	return ERR_OK;	
}



void SoundOutput::UpdateStreamSound(SOUND_ST *st)
{
	PROFILE();
	if (!m_mixer.IsVoiceON(st->mixerChannel))
	{
		CZLOG(LOG_INFO, "Stream not playing... Removing slot and disposing handle %u.\n", st->handle);
		st->streamSound->FinishedPlaying();
		st->streamSound = NULL;
	}
}

void SoundOutput::UpdateModuleSound(SOUND_ST *st)
{
	PROFILE();
	if (st->moduleSound->ReachedEnd()){
		CZLOG(LOG_INFO, "Module not playing... Removing slot and disposing handle %u.\n", st->handle);
		st->moduleSound = NULL;
	}
}

void SoundOutput::UpdateStaticSound(SOUND_ST *st)
{
	// If is not playing, then dispose slot
	if (!m_mixer.IsVoiceON(st->mixerChannel)) {
		CZLOG(LOG_INFO, "Sound not playing... Removing slot and disposing handle %u.\n", st->handle);
		st->staticSound = NULL;
	}
}

SOUND_ST* SoundOutput::GetEmptySlot(void)
{
	PROFILE();

	int count=m_maxActiveSounds;
	SOUND_ST *st= m_sounds;    
	while(count--){
		if (st->staticSound==NULL){
			memset(st, 0, sizeof(*st));
			return st;
		}       
		st++;   
	}   
	return NULL;    
}

SOUND_ST* SoundOutput::GetSlot(HSOUND sndHandle)
{
	PROFILE();

	if (sndHandle==0) return NULL;

	int count=m_maxActiveSounds;
	SOUND_ST *st= m_sounds;	
	while(count--){
		if ((st->handle==sndHandle)&&(st->staticSound!=NULL)){
			return st;
		}		
		st++;	
	}	
	return NULL;
}

void SoundOutput::UpdateStatus()
{	
	PROFILE();

	// Update the sounds status
	int i;
	SOUND_ST *st;
	for (i=0; i< m_maxActiveSounds;i++){
		st = &m_sounds[i];

		if ((st->staticSound!=NULL)&&(!st->paused))
		{
			switch (st->soundType)
			{
				case STREAMSOUND:
					UpdateStreamSound(st);
					break;
				case MODULESOUND:
					UpdateModuleSound(st);
					break;
				case STATICSOUND:
					UpdateStaticSound(st);
			};
			
			// If the pointer was set to NULL, then it finished playing
			if (st->staticSound==NULL && m_listener)
			{
				m_listener->OnPlayFinished(st->handle);
			}

		}

	}		
}

int SoundOutput::FeedData(void *ptr, int numFrames)
{

	uint8_t *mx=(uint8_t *)ptr;
	int mixPosBytes = 0;
	int frameSizeBytes = m_mixer.GetFrameSizeBytes();

	int part;
	int todo = numFrames;
	while (todo>0)
	{
		part = todo;
		// run through all the play requests, and update Modules
		int i;
		SOUND_ST *st;
		for (i=0; i< m_maxActiveSounds;i++)
		{
			st = &m_sounds[i];
			if ((st->staticSound!=NULL)&&(st->soundType==MODULESOUND)&&(!st->paused))
			{
				Module *mod = st->moduleSound;
				if (!mod->ReachedEnd())
				{
					// process tick if necessary
					if (st->bpmMixTodo<=0)
					{
						mod->DoTick();
						// Calculate number of frames to mix before the next tick
						st->bpmMixTodo= (125L*m_mixer.GetMixFrequency())/(50L*mod->GetBPM());
					}
					// keep saving the smaller step for next tick
					part = MIN(st->bpmMixTodo, part);
				}
			}
		}
	
		m_mixer.MixPortion(&mx[mixPosBytes], part);

		// Run trough all the Module play request, and update bpm information
		for (i=0; i< m_maxActiveSounds;i++){
			st = &m_sounds[i];
			if ((st->staticSound!=NULL)&&(st->soundType==MODULESOUND)&&(!st->paused)){
				st->bpmMixTodo -= part;
			}
		}
		
		todo -= part;
		mixPosBytes += (part * frameSizeBytes);
	}

	if (m_listener)
		m_listener->OnBufferMixed(ptr, numFrames);

	return ERR_OK;
}



/************************************************/
/************************************************/
/************************************************/
/************************************************/
/************************************************/









Module* SoundOutput::LoadModule(::cz::io::File *in)
{
	int err;
	Module *mod=NULL;

#if CZ_PLAYER_MOD_ENABLED
	if (MODModule::CheckFormat(in)){
		if ((mod = CZNEW(MODModule)(m_core))==NULL) CZERROR_RETNULL(ERR_NOMEM);
	}
#endif

#if CZ_PLAYER_IT_ENABLED
	if (ITModule::CheckFormat(in)) {
		if ((mod = CZNEW(ITModule)(m_core))==NULL) CZERROR_RETNULL(ERR_NOMEM);
	}
#endif

	if (mod==NULL) CZERROR_RETNULL(ERR_WRONGFORMAT);
	err = mod->Init(in);
	if (err!=ERR_OK){
		CZDELETE(mod);
		CZERROR_RETNULL(err);
	}

	return mod;
}


//
//
// czPlayerInterface methods
//
//


int SoundOutput::Update()
{
	return ERR_OK;
}

#if CZ_PLAYER_DISKFILE_ENABLED

HMODULEDATA SoundOutput::LoadModule(const char* filename)
{
	::cz::io::DiskFile in(m_core);
	int err = in.Open(filename, 0, ::cz::io::FILE_READ);
	if (err!=ERR_OK) CZERROR_RETNULL(err);
	Module *mod = LoadModule(&in);
	in.Close();
	return mod;
}
#endif


HMODULEDATA SoundOutput::LoadModule(const void* data, int dataSize)
{
	::cz::io::MemFile in(m_core);
	in.Open((uint8_t*)data, dataSize);
	Module *mod = LoadModule(&in);
	in.Close();
	return mod;
}


//
// WAV loading files
//
#if CZ_PLAYER_WAV_ENABLED

struct formatchunk
{
	char fID[4];
	uint32_t fLen;
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t FormatSpecific;
};
#define WAVE_FORMAT_PCM 1

StaticSound* SoundOutput::LoadWAV(::cz::io::File *in)
{
	int err=ERR_OK;
	StaticSound *tmpsnd = NULL;

	if (WAVLoader::CheckFormat(in))
	{
		WAVLoader loader(m_core);
		if ((err = loader.Load(in, &tmpsnd))!=ERR_OK)
			CZERROR_RETNULL(err);
	}

	if (tmpsnd==NULL)
	{
		CZLOG(LOG_ERROR, "Unrecognizable sound file format.\n");
		CZERROR_RETNULL(ERR_WRONGFORMAT);
	}

	return tmpsnd;
}

#if CZ_PLAYER_DISKFILE_ENABLED
HSOUNDDATA SoundOutput::LoadWAV(const char* filename)
{
	::cz::io::DiskFile in(m_core);
	int err;
	if ((err=in.Open(filename, 0, ::cz::io::FILE_READ))!=ERR_OK) CZERROR_RETNULL(err);	
	return LoadWAV(&in);
}
#endif

HSOUNDDATA SoundOutput::LoadWAV(const void *data, int dataSize)
{
	::cz::io::MemFile in(m_core);
	in.Open((uint8_t*)data,dataSize);	
	return LoadWAV(&in);
}

#endif // CZ_PLAYER_WAV_ENABLED

int SoundOutput::FreeModuleSound(HMODULEDATA handle)
{
	CZDELETE((Module*)handle);
	return ERR_OK;
}

int SoundOutput::FreeStaticSound(HSOUNDDATA handle)
{
	CZDELETE((StaticSound*)handle);
	return ERR_OK;
}

#if CZ_PLAYER_OGG_ENABLED

StreamSound* SoundOutput::LoadStream(::cz::io::File* in)
{
	StreamSound* stream = CZNEW(StreamSound)(m_core);
	int err = stream->Init(in, m_mixer.GetMaxOutputBufferSizeFrames()*2);
	if (err!=ERR_OK)
		CZERROR_RETNULL(err);

	return stream;
}

HMODULEDATA SoundOutput::LoadStream(const void* data, int dataSize)
{
	::cz::io::MemFile* in = CZNEW(::cz::io::MemFile)(m_core);
	void* ownmemory = CZALLOC(dataSize);
	memcpy(ownmemory, data, dataSize);
	in->Open(static_cast<uint8_t*>(ownmemory), dataSize, true);
	StreamSound* stream = LoadStream(in);
	return stream;
}
int SoundOutput::FreeStream(HSOUNDDATA handle)
{
	CZDELETE((StreamSound*)handle);
	return ERR_OK;
}

#if CZ_PLAYER_DISKFILE_ENABLED
HSTREAMDATA SoundOutput::LoadStream(const char* filename)
{
	// Load to memory
	::cz::io::DiskFile in(m_core);
	int err = in.Open(filename, 0, ::cz::io::FILE_READ);
	if (err!=ERR_OK)
		CZERROR_RETNULL(err);
	int fileSize = in.GetSize();
	void* ownmemory = CZALLOC(fileSize);
	err = in.ReadData(ownmemory, fileSize);
	if (err!=ERR_OK)
	{
		CZFREE(ownmemory);
		CZERROR_RETNULL(err);
	}

	::cz::io::MemFile* memfile = CZNEW(::cz::io::MemFile)(m_core);
	memfile->Open(static_cast<uint8_t*>(ownmemory), fileSize, true);

	StreamSound *stream = LoadStream(memfile);
	return stream;
}
#endif // CZ_PLAYER_DISKFILE_ENABLED
#endif // CZ_PLAYER_OGG_ENABLED


void SoundOutput::SetInterpolationMode(AudioInterpolationMode quality)
{
	LockMixer();
	m_mixer.SetQuality(quality);
	UnlockMixer();
}

AudioInterpolationMode SoundOutput::GetInterpolationMode()
{
	return m_mixer.GetQuality();
}

void SoundOutput::SetVolumeRamping(int numFrames)
{
	CZASSERT(numFrames>=0 && numFrames<=2048);
	LockMixer();
	m_mixer.SetVolumeRamping(numFrames);
	UnlockMixer();
}

int SoundOutput::GetVolumeRamping()
{
	return m_mixer.GetVolumeRamping();
}

void SoundOutput::SetStopRamping(int numFrames)
{
	CZASSERT(numFrames>=0 && numFrames<=2048);
	LockMixer();
	m_mixer.SetStopRamping(numFrames);
	UnlockMixer();
}
int SoundOutput::GetStopRamping()
{
	return m_mixer.GetStopRamping();
}

int SoundOutput::GetOutputFrequency(void){
	return m_mixer.GetMixFrequency();
}

int SoundOutput::GetMaxChannels(void){
	return m_mixer.GetChannels();
}

int SoundOutput::GetChannelsPlaying(void){
	int activeChannels=0;
	for (int ch=0; ch<m_mixer.GetChannels(); ch++){
		if (m_mixer.IsVoiceON(ch)) activeChannels++;
	}
	return activeChannels;
}

int SoundOutput::GetOutputChannels(void){
	return (m_mixer.IsStereo()) ? 2 : 1;
}

int SoundOutput::GetOutputBits(void){
	return (m_mixer.Is16Bits()) ? 16 : 8;
}


HSOUND SoundOutput::Play(HSOUNDDATA sndDataHandle, int vol, int pan, LoopMode loopMode, int loopStart, int loopLength)
{    
	PROFILE();

	StaticSound *snd = (StaticSound*) sndDataHandle;

	SOUND_ST *st;	
	if ((st=GetEmptySlot())==NULL){
		return -1;
	}

	HSOUND handle=0;

	// Validate parameters
	int panToUse, volToUse;
	if (pan==-1){
		panToUse = snd->GetPanning();
	} else {
		if (!INRANGE(pan, AUDIO_PAN_LEFT, AUDIO_PAN_RIGHT)) CZERROR(ERR_INVPAR);
		panToUse = pan;
	}
	if (vol==-1){
		volToUse = snd->GetVolume();
	} else {
		if (!INRANGE(vol, 0, AUDIO_VOL_MAX)) CZERROR(ERR_INVPAR);
		volToUse = vol;
	}

	int endPos;
	if (loopMode==SOUND_LOOP_OFF)
	{
		endPos = snd->GetNumFrames();
		loopStart = 0;
	}
	else
	{
		// Validate loopStart
		if (loopStart<0 || loopStart>=snd->GetNumFrames())
			loopStart = 0;

		// Validate the loop length
		if (loopLength<=0 || (loopStart+loopLength)>snd->GetNumFrames())
			endPos = snd->GetNumFrames();
		else
			endPos = loopStart+loopLength;
	}


	LockMixer();
	int ch = m_mixer.GetFreeChannel();	
	if (ch>=0)
	{
		handle = m_lastHandle+1;
		m_lastHandle++;
		m_mixer.SetVoice(ch, snd, 0, endPos, loopStart, snd->GetFrequency(), volToUse, panToUse, loopMode);
		m_mixer.SetVoiceStatus(ch, true);
		m_mixer.SetMasterVolume(m_sfxMasterVolume, ch,1);
		// keep values
		st->staticSound = snd;
		st->handle = handle;			
		st->soundType = STATICSOUND;
		st->mixerChannel = ch;
		CZLOG(LOG_INFO, "Playing sound with handle %u\n", handle);
	}
	
	// REMOVE THIS
	/*
	{
		czDiskFile out(parent);
		if (out.Open("c:\\system\\apps\\symbian_example_1\\out.raw",0, CZFILE_WRITE)!=ERR_OK)
			User::Panic(_L("czPlayer"),1);
		char buf[1000];
		int i=250;
		while(i--)
		{
			FeedData(buf,1000/2);
			out.WriteData(buf,1000);
		}
	}
	*/
	
	UnlockMixer();

	return handle;	
}

HSOUND SoundOutput::PlayModule(HMODULEDATA modDataHandle, int masterVol, bool loop, int firstOrder, int lastOrder)
{
	PROFILE();

	Module* mod = (Module*) modDataHandle;

	SOUND_ST *st;  
	if ((st=GetEmptySlot())==NULL){
		return -1;
	}

	masterVol = masterVol >> 2;
	// Call Start first, because it may fail if the mixer doesn't have enough free channels
	LockMixer();
	int err=mod->Start(&m_mixer, firstOrder, lastOrder, loop);
	if (masterVol!=AUDIO_MASTERVOL_DEFAULT){
		mod->SetMasterVolume(CLAMP(masterVol, 0, AUDIO_MASTERVOL_MAX));
	}
	UnlockMixer();
	if (err!=ERR_OK) return -1;

	HSOUND handle = m_lastHandle+1;
	m_lastHandle++;

	// keep values
	st->moduleSound = mod;
	st->handle = handle;            
	st->mixerChannel = -1;
	st->soundType = MODULESOUND;
	st->bpmMixTodo=0;

	CZLOG(LOG_INFO, "Playing module with handle %u\n", handle);
	return handle;
}

HSOUND SoundOutput::PlayStream(HSTREAMDATA sndDataHandle, int vol, bool loop)
{

	StreamSound* streamSnd = (StreamSound*) sndDataHandle;
	SOUND_ST *st;
	if ((st=GetEmptySlot())==NULL)
	{
		return -1;
	}

	HSOUND handle = 0;

	int volToUse;
	if (vol==-1){
		volToUse = streamSnd->GetVolume();
	} else {
		if (!INRANGE(vol, 0, AUDIO_VOL_MAX)) CZERROR(ERR_INVPAR);
		volToUse = vol;
	}

	LockMixer();
	if (!streamSnd->IsPlaying())
	{
		int ch = m_mixer.GetFreeChannel();	
		if (ch>=0)
		{
			handle = m_lastHandle+1;
			m_lastHandle++;
			m_mixer.SetVoice(ch, streamSnd, volToUse, loop);
			m_mixer.SetVoiceStatus(ch, true);
			m_mixer.SetMasterVolume(m_sfxMasterVolume, ch,1);
			// keep values
			st->streamSound= streamSnd;
			st->handle = handle;			
			st->soundType = STREAMSOUND;
			st->mixerChannel = ch;
			CZLOG(LOG_INFO, "Playing stream with handle %u\n", handle);
		}
	}
	UnlockMixer();

	return handle;
}

int SoundOutput::Stop(HSOUND sndHandle)
{
	PROFILE();

	// Find the slot with the sound
	SOUND_ST *slot = GetSlot(sndHandle);
	if (slot==NULL) CZERROR(ERR_INVPAR);

	if (slot->soundType==MODULESOUND){
		slot->moduleSound->Stop();
		slot->moduleSound=NULL;
	} else if ((slot->soundType==STATICSOUND)||(slot->soundType==STREAMSOUND)) {
		// Stop the sound
		LockMixer();
		m_mixer.SetVoiceStatus(slot->mixerChannel, false);
		if (slot->soundType==STREAMSOUND)
			slot->streamSound->FinishedPlaying();
		UnlockMixer();
		slot->staticSound=NULL;	
	}

	return ERR_OK;
}

int SoundOutput::StopAll(void)
{
	PROFILE();

	LockMixer();

	int count=m_maxActiveSounds;
	SOUND_ST *slot= m_sounds;    
	while(count--){
		if (slot->staticSound!=NULL){
			if (slot->soundType==MODULESOUND){
				slot->moduleSound->Stop();
				slot->moduleSound=NULL;
			} else if ((slot->soundType==STATICSOUND)||(slot->soundType==STREAMSOUND)) {
				m_mixer.SetVoiceStatus(slot->mixerChannel, false);
				if (slot->soundType==STREAMSOUND)
					slot->streamSound->FinishedPlaying();
				slot->staticSound = NULL;
			}
		}       
		slot++;   
	}   

	UnlockMixer();

	return ERR_OK;
}

int SoundOutput::SetVolume(HSOUND sndHandle, uint8_t vol)
{
	PROFILE();

	// Find the slot with the sound
	SOUND_ST *slot = GetSlot(sndHandle);
	if (slot==NULL) CZERROR(ERR_INVPAR);

	LockMixer();
	if (slot->soundType==MODULESOUND){
		slot->moduleSound->SetMasterVolume(vol >> 2);
	} else if ((slot->soundType==STATICSOUND)||(slot->soundType==STREAMSOUND)) {
		m_mixer.SetVolume(slot->mixerChannel, vol);
	}
	UnlockMixer();

	return ERR_OK;
}

int SoundOutput::SetFrequency(HSOUND sndHandle, int freq)
{
	PROFILE();

	// Find the slot with the sound
	SOUND_ST *slot = GetSlot(sndHandle);
	if (slot==NULL) CZERROR(ERR_INVPAR);

	// You can't change the playing frequency of songs
	if (slot->soundType==MODULESOUND) CZERROR(ERR_INVPAR);

	// Set volume
	LockMixer();
	m_mixer.SetFrequency(slot->mixerChannel, freq);
	UnlockMixer();

	return ERR_OK;	
}

int SoundOutput::SetPanning(HSOUND sndHandle, uint8_t pan)
{
	PROFILE();

	// Find the slot with the sound
	SOUND_ST *slot = GetSlot(sndHandle);
	if (slot==NULL) CZERROR(ERR_INVPAR);

	// You can't change the panning of songs
	if (slot->soundType==MODULESOUND) CZERROR(ERR_INVPAR);

	// Set volume
	LockMixer();
	m_mixer.SetPanning(slot->mixerChannel, pan);
	UnlockMixer();

	return ERR_OK;
}

int SoundOutput::Pause(HSOUND sndHandle)
{
	// Find the slot with the sound
	SOUND_ST *st = GetSlot(sndHandle);
	if (st==NULL) CZERROR(ERR_INVPAR);

	if (st->paused) return ERR_OK;

	if (st->soundType==MODULESOUND){
		st->moduleSound->Pause();
	} else if ((st->soundType==STATICSOUND)||(st->soundType==STREAMSOUND)){
		LockMixer();
		m_mixer.SetVoiceStatus(st->mixerChannel, false);
		UnlockMixer();
	}
	st->paused = true;
	return ERR_OK;
}

int SoundOutput::Resume(HSOUND sndHandle)
{
	// Find the slot with the sound
	SOUND_ST *st = GetSlot(sndHandle);
	if (st==NULL) CZERROR(ERR_INVPAR);

	if (!st->paused) return ERR_OK;

	if (st->soundType==MODULESOUND){
		st->moduleSound->Resume();
	} else if ((st->soundType==STATICSOUND)||(st->soundType==STREAMSOUND)){
		LockMixer();
		m_mixer.SetVoiceStatus(st->mixerChannel, true);
		UnlockMixer();
	}

	st->paused = false;
	return ERR_OK;
}

AudioPlayerListener* SoundOutput::SetListener(AudioPlayerListener *listener)
{
	AudioPlayerListener* prev = m_listener;
	m_listener = listener;
	return prev;
}

int SoundOutput::SetSFXMasterVolume(int vol)
{
	if (!INRANGE(vol, 0, AUDIO_MASTERVOL_MAX)) CZERROR(ERR_INVPAR);
	m_sfxMasterVolume = vol;
	return ERR_OK;
}

int SoundOutput::GetSFXMasterVolume(void)
{
	return m_sfxMasterVolume;
}

/*
void* SoundOutput::GetParent()
{
	return m_core;
}
*/



void SoundOutput::Destroy()
{
	//
	// TODO - Deleting like this is dangerous. If we don't cast to the right type (one that really implements czPlayerInterface), it will mess up the vtables
	CZDELETE(this);
}


//
// This function needs to be at the bottom of the file, because we redefine COREOBJ inside
//
AudioPlayer* AudioPlayer::Create(Core *core, AudioPlayerConfig *cfg)
{
	AudioPlayerConfig cfgToUse;

	GetDefaultConfig(&cfgToUse);

	// If a configuration was specified, we override the defaults with the user provided values
	if (cfg!=0)
	{
		if (cfg->driverType!=AUDIO_DRIVER_DEFAULT)
			cfgToUse.driverType = cfg->driverType;
		if (cfg->bufSizeMs!=0)
			cfgToUse.bufSizeMs = cfg->bufSizeMs;
		if (cfg->frequency!=0)
			cfgToUse.frequency = cfg->frequency;
		if (cfg->bits!=0)
			cfgToUse.bits = cfg->bits;
		if (cfg->outputChannels!=0)
			cfgToUse.outputChannels = cfg->outputChannels;
		if (cfg->maxChannels!=0)
			cfgToUse.maxChannels = cfg->maxChannels;
		if (cfg->interpolationMode != AUDIO_INTERPOLATION_DEFAULT)
			cfgToUse.interpolationMode = cfg->interpolationMode;
	}

	// Change the way we access the core for the rest of the code
#undef COREOBJ
#define COREOBJ core

	//
	// configuration on a platform basis
	//
#if CZ_PLATFORM_WIN32
	SoundOutput* out = NULL;
	if (cfgToUse.driverType==AUDIO_DRIVER_WINMM)
	{
		out = CZNEW(Win32WaveOutOutput)(core);
	}
	else
	{
		CZLOG(LOG_ERROR,"Invalid output driver specified");
		CZERROR_RETNULL(ERR_INVPAR);
	}
#elif CZ_PLATFORM_ARDUINO
	if (cfgToUse.driverType!=AUDIO_DRIVER_ARDUINO_I2S)
	{
		CZLOG(LOG_ERROR, "Invalid output driver specified");
		CZERROR_RETNULL(ERR_INVPAR);
	}
	ArduinoI2SOutput* out = CZNEW(ArduinoI2SOutput)(core);
#else
	#error No platform specific code specified
#endif

	if (out==NULL)
		CZERROR_RETNULL(ERR_NOMEM);

	// #TODO: I should code a more robust configuration validation, because some platforms require certain frequencies, bits, etc

	int err=out->Init(cfgToUse.maxChannels, cfgToUse.bufSizeMs, (cfgToUse.outputChannels==2) ? true : false, (cfgToUse.bits==16) ? true : false, cfgToUse.frequency);
	if (err!=ERR_OK)
	{
		CZLOG(LOG_ERROR,"Error initializing audio device\n");
		CZDELETE(out);
		CZERROR_RETNULL(err);
	}

	out->SetInterpolationMode(cfgToUse.interpolationMode);

	CZLOG(LOG_INFO, "Output device created\n");

	return out;

}

} // namespace microaudio
} // namespace cz

