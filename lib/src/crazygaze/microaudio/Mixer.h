//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
//  Class responsible for software mixing
//

#ifndef _CZMIXER_H_
#define _CZMIXER_H_

#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/Audio.h>
#include <crazygaze/microaudio/MixerListener.h>
#include <crazygaze/microaudio/Object.h>

namespace cz
{
namespace microaudio
{

// forward declarations
class StaticSound;
class StreamSound;

			/* SKIP THIS */
#ifndef DOXYGEN_SKIP

//#define ASM_MIXER
/*
#ifdef ASM_MIXER
	#pragma aux push_args parm caller [];
	#pragma aux (MXFUNC, push_args);
#endif
*/

typedef struct 
{
	int32_t currLvol;
	int32_t destLvol;
	int32_t currRvol;
	int32_t destRvol;
	int32_t incLvol;
	int32_t incRvol;
	int volrampcount;
	
	int32_t lastSampleMixedL;
	int32_t lastSampleMixedR;

	bool restart;
} VOLUME_STATE;

typedef struct  
{
	int32_t currLval;
	int32_t currRval;
	int32_t incLval;
	int32_t incRval;
	int32_t stoprampcount;
	bool restart;
} STOPRAMP_STATE;

typedef uint32_t (MXFUNC)(int32_t *dest,void *src, int32_t pos ,int32_t inc,
						 VOLUME_STATE *volstate, int32_t portion);

typedef struct
{
	volatile int pos;	// position which is currently playing
				// with fractbits
	int repeat;		// goes here after hitting  end
	int end;		// position of last sample+1
	uint32_t freq;		// playing frequency in hertz
	StaticSound *snd;		// pointer to current wave	
	uint8_t vol;              // Note volume, ranges from 0 to 255
	uint8_t mastervol;        // 0-64 = whole volume range of output
	uint8_t panning;          // 0 left, 128 middle, 255 right
	int lloop;		// loop mode
	uint8_t on;		// on ???       
                
	uint8_t back;     // is it going back (ping-pong loop!!!)
	int32_t increment;// increment (samplefrequency/mixfrequency)

	MXFUNC *mixfunc;

	uint32_t tag; // custom tag, for anything the user may need
	
	// tells if the channel is reserved. This is used for when playing songs.
	// A songs needs to use consecutive channels, so we must mark the channels requested by a song.
	uint8_t reserved; 

	VOLUME_STATE volState;
	STOPRAMP_STATE stopRampState;

	// used to implement streaming
	ChannelMixingListener* mixingListener;

} CHANNEL;

#endif


class Mixer : public ::cz::Object
{
	
public:
	Mixer(::cz::Core *parentObject);
	virtual ~Mixer();
	int Init(uint32_t numberofchannels, uint32_t mixsize, bool stereo, bool is16bits, uint16_t freq);
	void Free(void);
	int MixPortion(void *dest, uint32_t len);

	void SetVolumeRamping(int numFrames);
	int GetVolumeRamping()
	{
		return volumeRampingNumFrames;
	}
	void SetStopRamping(int numFrames);
	int GetStopRamping()
	{
		return stopRampingNumFrames;
	}

	int SetQuality(AudioInterpolationMode qtype);
	AudioInterpolationMode GetQuality();

	/* interface functions */
	int SetVoice(uint32_t ch, StaticSound *sound,
			 uint32_t current, uint32_t end, uint32_t repeat,
			 uint32_t freq, uint8_t vol, uint8_t pan, int loopmode);
	int SetVoice(uint32_t ch, StreamSound *stream, uint8_t vol, bool loop);
	int SetSample(int ch, StaticSound *snd,uint32_t current, uint32_t end, uint32_t repeat,  int loopmode);

	int SetPosition(int ch, uint32_t current);
	int SetLoop(uint8_t ch, uint32_t loopbeg, uint32_t loopend, int loopmode);
	int SetFrequency(int ch, uint32_t freq);
	int SetVolume(int ch, uint8_t vol);
	int SetPanning(int ch, uint8_t pan);
	int SetVoiceStatus(int ch, int on);

	int SetMixingListener(int ch, ChannelMixingListener* listener);

	void SetMasterVolume(uint8_t v);	
	
	int SetMasterVolume(uint8_t v,uint32_t firstchannel,uint32_t howmany);	
	int GetFreeChannel(void);
    
	int IsVoiceON(int ch);
    
	int GetMasterVolume(int ch);

	int ReserveChannels(int numChannels);
	void FreeChannels(int firstChannel, int numChannels);
	int ReserveSingleChannel(int channel);
	int FreeChannel(int channel);

	uint32_t GetChannelTag(int ch);
	void SetChannelTag(int ch, uint32_t tag);
	
	// Returns the buffer size in bytes
	uint32_t GetMaxOutputBufferSizeBytes();
	// Return the buffer size in frames
	uint32_t GetMaxOutputBufferSizeFrames();

	int GetFrameSizeBytes(void);
	int GetMixFrequency(void);
	bool IsStereo(void);
	bool Is16Bits(void);
	int GetChannels(void);
		
#if CZ_PLAYER_EXTRAFUNCTIONS_ENABLED	
	int GetPosition(int ch);
	int GetEnd(int ch);
	int GetRepeat(int ch);
//	int GetLoopMode(int ch,int *lm);
	int GetFrequency(int ch);
	int GetVolume(int ch);
	int GetPanning(int ch);
	int GetVoiceValue(int ch);
	int IsVoiceGoingBack(int ch);
#endif

	enum
	{
		// Precision (in bits) used for frequency in the mixing.
		FREQFRACBITS=10,
		// Mask to get the frequency bits.
		FREQFRACMASK=((1L<<FREQFRACBITS)-1),
		//! Number of bits used for volume.
		VOLFRACBITS=14
	};

	enum
	{
		AUDIO_MIXER_MAXSAMPLE_SIZE = (1 << (31-FREQFRACBITS))-1
	};

private:

	void MakeOutput(uint8_t *dest,uint32_t todo);
	void MakeOutput(uint16_t *dest,uint32_t todo);
	void MixChannel(CHANNEL *chptr,uint32_t len, int32_t *mixbuff);

	void SetStopRamp(CHANNEL *chptr);
	void Mix_StopRamp(int32_t *dest, STOPRAMP_STATE *stoprampstate, int32_t portion);


	// Mixer variable members

	// Buffer used for mixing
	int32_t *m_mixblock;

	CHANNEL *channels;
	uint32_t samples; // blocksize in nº of DWORDS, not BYTES
	uint32_t samplesize; // DWORDS per sample	
	int nch;	// number of channels
	bool IS_16BITS; // 8, or 16 bits
	bool IS_STEREO; // is stereo ?
	uint32_t FREQUENCY; // mixing frequency
	int volumeRampingNumFrames;
	int stopRampingNumFrames;
	AudioInterpolationMode QualityType;
	MXFUNC *Mix16StereoFunc;
	MXFUNC *Mix8StereoFunc;
	MXFUNC *Mix16MonoFunc;
	MXFUNC *Mix8MonoFunc;
};


} // namespace microaudio
} // namespace cz



#endif
