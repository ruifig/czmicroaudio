//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZMODMODULE_H_
#define _CZMODMODULE_H_

#include "czPlayerPrivateDefs.h"
#include "czModule.h"
#include "czAudio.h"

// forward declarations
namespace cz
{
	namespace io
	{
		class File;
	}
}


namespace cz
{
namespace audio
{


// forward declarations
class StaticSound;

enum 
{
	MOD_SONGNAMELENGTH=20,
	MOD_SAMPLENAMELENGTH=22,
	MOD_NUMSAMPLES=31,
	MOD_NUMORDERS=128
};

struct MODSAMPLE {
	char name[MOD_SAMPLENAMELENGTH];
	int length; // sample length in bytes
	s8 fineTune; // sample initial finetune (-8..7)
	s8 volume; // 0..64
	int loopStart;
	int loopLength; // only loop if this value is > 2
	StaticSound *snd;
};


struct MODPACKEDNOTE {
	// Note used 7 bits (0..123), sample uses 5 bits (0..31), effect uses 4 bits (0..15), and effect parameter 8 bits (0..255)
	// N - Note bits, S - sample bit , E-effect bit, P-effect parameter bit
	//             76543210
	u8 b0; // NNNNNNNS
	u8 b1; // SSSSEEEE
	u8 b2; // PPPPPPPP
};

struct MODNOTE{
	s16 periodIndex;
	u8 sample;
	u8 effect;
	u8 effectParam;
};

struct MODTRACK {
	MODNOTE note; // Current row note's info
	u8 sampleNum; // 1..MAX_SAMPLES
	MODSAMPLE *samplePtr;

	u8 currentVolume; // 0..64 current channel volume, with slides
	u8 volumeToPlay; // 0..64 volume to use, with tremolo, etc.

	u8 panning; // 0-left 64-middle 128-right

	int originalPeriodIndex; // Original period index, when there is a note entered
	int currentPeriod; // Current period, with slides.
	int periodToPlay; // period to play, with all effects applied, etc

	// Portamento variables
	int portamentoTargetPeriod; // target period when there is a portamento
	s8 periodDirection; // -1= period is decreasing   , 1=period is increasing
	u8 portamentoSpeed; // memory of portamento effect
	bool glissando; // Tells if glissando is active for this channel

	// vibrato variables
	u8 vibratoSpeed;
	u8 vibratoDepth;
	u8 vibratoWavePos;
	u8 vibratoWaveform; // may be 0,1,2,3
	bool vibratoRetrig;

	// tremolo variables
	u8 tremoloSpeed;
	u8 tremoloDepth;
	u8 tremoloWavePos;
	u8 tremoloWaveform; // may be 0,1,2,3
	bool tremoloRetrig;

	// Set sample offset memory
	u8 sampleOffset;

	int mixerChannelUsed;

	// Arpeggio variables
	s8 arpeggioCounter; // has values from 0..2


	bool flagSetSample;
	bool flagResetSample;

	// Used to support pause. We must remember what mixer channels were active before the pause, so that we can reactivate them
	// After a pause
	bool wasOnBeforePause;
};

class MODModule : public Module {

public:

	//! Default constructor
	MODModule(::cz::Core *parentObject);
	//! Destructor
	virtual ~MODModule();

	// Load the song
	int Init(::cz::io::File *in);
	static bool CheckFormat(::cz::io::File *in);

	int Start(Mixer *mixer, int firstOrder, int lastOrder, bool loop, u8 volume=AUDIO_MASTERVOL_DEFAULT);
	int Stop(void);
	int Pause(void);
	int Resume(void);

	//int GetFirstMixerChannel(void);
	int GetChannels(void);
	int GetPosition(void);
	int GetOrders(void);
	int GetPattern(void);
	int GetPatternRows(void);
	int GetRow(void);
	int GetSpeed(void);

protected:
	friend class czSoundOutput;
	virtual void DoTick(void);
	virtual int SetMasterVolume(u8 vol);

private:
	bool m_isPlaying;
	bool m_loaded;
	bool m_isPaused;
	char m_songName[MOD_SONGNAMELENGTH];
	int m_numChannels;
	MODSAMPLE m_samples[MOD_NUMSAMPLES];
	u8 m_orders[MOD_NUMORDERS];
	u8 m_numOrders; // Number of orders
	int m_numPatterns; // Number of patterns present in the song

	u8 *m_tempPatternData;
	MODPACKEDNOTE *m_packedPatternData;
	MODTRACK *m_tracks;

	int m_firstMixerChannel; // Tells the first channel used from the mixer.
	int m_speed; // Speed of the song
	int m_currentTick; // the tick currently playing
	int m_currentRow; // current playing row (0..63)
	int m_currentOrder; // playing order position
	int m_firstOrder; // where the playback started. This is needed so we know where to jump to if we want to play pattern range and loop it
	int m_lastOrder; // where to finish playing, if we want to play until a particular order instead of the end

	int m_jumpToOrder; // -1 = no jump  >=0 jump to that order
	int m_patternBreakToRow; // -1 = no pattern break   , >=0 pattern break to that row

	s8 m_patternLoopRow; // Tells the row position to loop back when using effect E6x
	s8 m_patternLoopCount;
	s8 m_patternLoopTo; // -1 = no loop ,  >=0 loop to that row

	int m_patternDelay; // 0 = no delay , >0 delay

	void CleanUpMemory(void);
	void ResetMembers(void);
	void DecodeRow(void); // decodes current row (m_currentRow);
	void ProcessRow(void);
	void ProcessFirstTickEffects(void);
	void UpdateEffects(void);
	void UpdateMixer(void);
	//void SetPlayingPeriodByIndex(MODTRACK *trk, int periodIndex);
	//void SetCurrentPeriod(MODTRACK *trk, int periodIndex);
	int CalculatePeriod(MODTRACK *trk, int periodIndex, int periodIncrement=0);
	int CalculateGlissandoSemitone(MODTRACK *trk, int period);

	void DoVibrato(MODTRACK *trk, bool vibratoAndVolSlide);
	void DoTremolo(MODTRACK *trk);
	void DoTonePortamento(MODTRACK *trk, bool tonePortamentoAndVolSlide);
	void DoVolumeSlide(MODTRACK *trk);
	void DoPortamentoUp(MODTRACK *trk, bool fine=false);
	void DoPortamentoDown(MODTRACK *trk, bool fine=false);
};

} // namespace audio
} // namespace cz

#endif
