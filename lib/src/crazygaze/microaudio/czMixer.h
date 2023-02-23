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

#include "czPlayerPrivateDefs.h"
#include "czAudio.h"
#include "czMixerListener.h"
#include "czObject.h"

namespace cz
{
namespace audio
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
	s32 currLvol;
	s32 destLvol;
	s32 currRvol;
	s32 destRvol;
	s32 incLvol;
	s32 incRvol;
	int volrampcount;
	
	s32 lastSampleMixedL;
	s32 lastSampleMixedR;

	bool restart;
} VOLUME_STATE;

typedef struct  
{
	s32 currLval;
	s32 currRval;
	s32 incLval;
	s32 incRval;
	s32 stoprampcount;
	bool restart;
} STOPRAMP_STATE;

typedef u32 (MXFUNC)(s32 *dest,void *src, s32 pos ,s32 inc,
						 VOLUME_STATE *volstate, s32 portion);

typedef struct
{
	volatile int pos;	// position which is currently playing
				// with fractbits
	int repeat;		// goes here after hitting  end
	int end;		// position of last sample+1
	u32 freq;		// playing frequency in hertz
	StaticSound *snd;		// pointer to current wave	
	u8 vol;              // Note volume, ranges from 0 to 255
	u8 mastervol;        // 0-64 = whole volume range of output
	u8 panning;          // 0 left, 128 middle, 255 right
	int lloop;		// loop mode
	u8 on;		// on ???       
                
	u8 back;     // is it going back (ping-pong loop!!!)
	s32 increment;// increment (samplefrequency/mixfrequency)

	MXFUNC *mixfunc;

	u32 tag; // custom tag, for anything the user may need
	
	// tells if the channel is reserved. This is used for when playing songs.
	// A songs needs to use consecutive channels, so we must mark the channels requested by a song.
	u8 reserved; 

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
	int Init(u32 numberofchannels, u32 mixsize, bool stereo, bool is16bits, u16 freq);
	void Free(void);
	int MixPortion(void *dest, u32 len);

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
	int SetVoice(u32 ch, StaticSound *sound,
			 u32 current, u32 end, u32 repeat,
			 u32 freq, u8 vol, u8 pan, int loopmode);
	int SetVoice(u32 ch, StreamSound *stream, u8 vol, bool loop);
	int SetSample(int ch, StaticSound *snd,u32 current, u32 end, u32 repeat,  int loopmode);

	int SetPosition(int ch, u32 current);
	int SetLoop(u8 ch, u32 loopbeg, u32 loopend, int loopmode);
	int SetFrequency(int ch, u32 freq);
	int SetVolume(int ch, u8 vol);
	int SetPanning(int ch, u8 pan);
	int SetVoiceStatus(int ch, int on);

	int SetMixingListener(int ch, ChannelMixingListener* listener);

	void SetMasterVolume(u8 v);	
	
	int SetMasterVolume(u8 v,u32 firstchannel,u32 howmany);	
	int GetFreeChannel(void);
    
	int IsVoiceON(int ch);
    
	int GetMasterVolume(int ch);

	int ReserveChannels(int numChannels);
	void FreeChannels(int firstChannel, int numChannels);
	int ReserveSingleChannel(int channel);
	int FreeChannel(int channel);

	u32 GetChannelTag(int ch);
	void SetChannelTag(int ch, u32 tag);
	
	// Returns the buffer size in bytes
	u32 GetMaxOutputBufferSizeBytes();
	// Return the buffer size in frames
	u32 GetMaxOutputBufferSizeFrames();

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

	void MakeOutput(u8 *dest,u32 todo);
	void MakeOutput(u16 *dest,u32 todo);
	void MixChannel(CHANNEL *chptr,u32 len, s32 *mixbuff);

	void SetStopRamp(CHANNEL *chptr);
	void Mix_StopRamp(s32 *dest, STOPRAMP_STATE *stoprampstate, s32 portion);


	// Mixer variable members

	// Buffer used for mixing
	s32 *m_mixblock;

	CHANNEL *channels;
	u32 samples; // blocksize in nº of DWORDS, not BYTES
	u32 samplesize; // DWORDS per sample	
	int nch;	// number of channels
	bool IS_16BITS; // 8, or 16 bits
	bool IS_STEREO; // is stereo ?
	u32 FREQUENCY; // mixing frequency
	int volumeRampingNumFrames;
	int stopRampingNumFrames;
	AudioInterpolationMode QualityType;
	MXFUNC *Mix16StereoFunc;
	MXFUNC *Mix8StereoFunc;
	MXFUNC *Mix16MonoFunc;
	MXFUNC *Mix8MonoFunc;
};


} // namespace audio
} // namespace cz



#endif
