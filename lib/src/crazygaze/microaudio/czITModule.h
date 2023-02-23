//
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// CrazyGaze Studios (www.crazygaze.com)
//

#ifndef _CZITMODULE_H_
#define _CZITMODULE_H_

#include "czPlayerPrivateDefs.h"
#include "czModule.h"

namespace cz
{

namespace audio
{

// forward declarations
class StaticSound;

typedef s32 fixed16_16;

			/* SKIP THIS DOCUMENTATION */
#ifndef DOXYGEN_SKIP
	
/****************************************************************************
  -------------------------------------------------------------------------
			FILE RELATED STRUCTURES
  -------------------------------------------------------------------------
 ****************************************************************************/

typedef struct
{
        char id[4]; // "IMPM"
        char modname[26];
        char notused[2];
        unsigned short OrdNum,InsNum,SmpNum,PatNum,CwtV,Cmwt;
        unsigned short Flags;
        unsigned short Special; // bit 0=ON -> Message at offset MessageOffset
        unsigned char GV,MV,IS,IT,Sep;
        char value0;
        unsigned short MsgLength;
        int MsgOffset;
        char reserved[4];
        unsigned char ChnlPan[64]; //panning for each channel
        			   // 0(left) 64(right) 32(center) 100(surround)	
        			   // +128(disable channel) : but effects are still processed
	unsigned char ChnlVol[64]; //volume for each channel
} IT_HEADER; // SIZE=192


typedef struct{
	unsigned char Flg; // Bit 0 : Envelope on/off  1=on 0=off
			   // Bit 1 : Loop on/off  1=on
			   // Bit 2 : SusLoop on/off  1=on
	unsigned char Num; // number of node points	
	unsigned char LpB; // Loop beginning
	unsigned char LpE; // Loop end
	unsigned char SLB; // Sustain loop beginning
	unsigned char SLE; // Sustain loop end	
	struct{
		signed char y;// vol(0-64)  pan/pitch(-32->+32)
		unsigned short tick; // tick number (0->9999)
		} Nodes[25];
	char unused;	
}IT_ENVELOPE; // SIZE=82

typedef struct
{
        unsigned char ID[4];
        char FileName[12]; // dos filename
        unsigned char value_00h;
        unsigned char NNA,DCT,DCA;
        unsigned short FadeOut;
        signed char PPS; // pitch pan seperation -31 -> + 32
        unsigned char PPC; // Pitch pan center   C-0 to B-9 as 0->119(inc)
        unsigned char GbV; // Global Volume
        unsigned char DfP; // default pan, 0->64, &128=>dont use
        unsigned char RV,RP; // random volume/panning variation
        unsigned short TrkVers; // only used in instrument files
        unsigned char NoS; // only used in instrument files
        char unused;
        char InsName[26]; // instrument name
        char notused[2];        
	unsigned char MCh,MPr; // Midi stuff        
	unsigned short MIDIBnk;
	struct{
		unsigned char note, sample;
	} KybdTbl[120]; // 0->199(inc)
	IT_ENVELOPE VolEnv,PanEnv,PitEnv;	
} IT_INSTRUMENT;


typedef	struct{
	char ID[4]; //"IMPS"
        char FileName[12]; // dos filename
        unsigned char value_00h;
        unsigned char GvL,Flg,Vol;
        char SmpName[26];
        unsigned char Cvt; // bit 0:   off: samples are unsigned   on=signed      
        unsigned char DfP; // default panning
        		// bit 0->6 = pan value
        		// bit 7(on)= to USE        
        int Length; // sample length (n§ of samples, not n§ of bytes)
        int LoopBeg; // (no§ of samples in, not bytes)
        int LoopEnd; // sample n§. AFTER end of loop
        int C5Speed; // Number of bytes a second for C-5 (0->9999999)
        int SusLBeg; // start of sustain loop
        int SusLEnd; // sample n§. AFTER end of sustain Loop
        unsigned int SmpPtr; // offset of sample in file
        unsigned char ViS,ViD; // vibrato speed/depth (0->64)
        unsigned char ViR; // rate at which vibrato is applied (0->64)
        unsigned char ViT; // vibrato waveform type
        		// 0-sinewave
        		// 1-rampdown
        		// 2-squaredown
        		// 3-random (speed is irrelevant)
}IT_SAMPLE;

	

typedef struct{
	unsigned short Length; // length of pattern data
	unsigned short Rows; // number of rows in pattern
	unsigned char *data; // pattern data in packed format
} IT_PATTERN;




typedef struct{
	unsigned char isnote; // check if note field as valid info
        unsigned char note; // 0-199
        unsigned char instrument; // 1-99
        unsigned char isvoleffect;
        unsigned char voleffect;
        unsigned char effect;
        unsigned char effectdata;
}IT_NOTE;

typedef struct
	{
	unsigned char lastmask;
	unsigned char lastnote;
	unsigned char lastinstrument;
	unsigned char lastvolpan;
	unsigned char lastcommand;
	unsigned char lastcommanddata;
	} IT_CHANNEL_LAST_INFO;

typedef struct{
	/*
	int currenttick,repeattick,endtick,loop;
	int repeatnode;
	int nextnode;
	*/
	
	int loop;
	int currentNode;
	int repeatNode;
	int endNode;
	int ticksToNextNode;
	
	fixed16_16 inc_; // Fixed Point 16.16
	fixed16_16 val_; // Fixed Point 16.16 (-32..32 to Pitch/Pan, 0..64 to Vol)
	int flag; // -1 restart // 0-off // 1 - running
	} ENVCONTROL;



typedef struct
{
	unsigned char host;
	unsigned char isdirect; // is it controled directly, or virtual ??

	unsigned char keyon;
	unsigned char active;
	unsigned char turnON;

	IT_INSTRUMENT *inst; // pointer to instrument
	IT_SAMPLE *smp;    // pointer to IT sample
	StaticSound *wv;	   // pointer to wave data
	
	u8 note;
	int insnum; // instrument number (1...)
	int smpnum; // sample number (1....)

	int VirtualChannelVol;
	int VirtualNoteVol;
	int VirtualNotePan;

	int NoteFreq; // integer
	
	int TargetFreq; // integer
	int FinalVol;
	int FinalFreq; // integer
	int FinalPan;
	

	unsigned short SampleVibratoAcum;
	u8 samplevibpos;
	
	ENVCONTROL VolEnv,PanEnv,PitEnv;
	unsigned char UseVolEnv,UsePanEnv,UsePitEnv;
	int NFC;
	int fadeout;
	bool paused;
} VIRTUALCHANNEL;


typedef struct{	
	int dumb;
	int vchannel;
	
	unsigned char active;		
	unsigned char kick;
	unsigned char OnlyKickVolume;
	
	u8 note;
	int insnum; // instrument number (1...)
	int smpnum; // sample number (1....)
	u8 isvoleffect;
	u8 voleffect;
	u8 effect;
	u8 effectdata;
	u8 instpresent;
	u8 realnote;


	int ChannelVol;
	int ChannelPan;	
	int NoteVol;
	int NotePan;
	int InitialNoteVol;
	
	u8 NNA;
	u8 DCT;
	u8 DCA;
	

	fixed16_16 VibFreqMul;

	unsigned char Vvolslide_mem;
	unsigned char volumeslide_mem;
	unsigned char panningslide_mem;
	
	unsigned char istoneportamento;
	unsigned char toneportamento_mem;
	unsigned char pitchslide_mem;
	unsigned char vibrato_mem;

	unsigned char isvibrato;
	unsigned char vibpos;
	unsigned char vibratowaveform;

	unsigned char globalvolumeslide_mem;	

	unsigned char istremor;
	unsigned char tremor_mem;
	unsigned char tremorOFFTIME;
	unsigned char tremorcount;

	unsigned char isarpeggio;
	unsigned char arpeggio_mem;
	unsigned char arpeggiocount;
	fixed16_16 ArpeggioMul; // Fixed 16.16
	
	unsigned char Seffect_mem;
	unsigned char retrigg_mem;
	int retriggcount;

	unsigned char tremolowaveform;
	unsigned char panbrellowaveform;

	unsigned int sampleoffset;
	unsigned char cutcountdown;

	int loopback,loopbackrow,loopbackcount;
	int notedelay;
	unsigned char channelvolumeslide_mem;
	unsigned char tremolo_mem;
	unsigned char trempos;
	int TremoloAdd;
	u8 istremolo;
	unsigned char settempo_mem;

	unsigned char panbrello_mem;
	int PanbrelloAdd;
	unsigned char panbrellopos;
	u8 ispanbrello;
} TRACK;


enum
{
	SMP_IT_SAMPLE=1,
	SMP_IT_16BIT=2,
	SMP_IT_STEREO=4, // not supported by IT yet
	SMP_IT_COMPRESSED=8, // IT 2.14 compressed sample
	SMP_IT_LOOP=16,
	SMP_IT_SUSTAINLOOP=32,
	SMP_IT_PINGPONGLOOP=64,
	SMP_IT_PINGPONGSUSTAINLOOP=128
};

enum
{
	MAX_IT_SAMPLES=200,
	MAX_IT_INSTRUMENTS=200,
	MAX_IT_PATTERNS=200
	//#define MAX_VIRTUAL_CHANNELS 64
};


		/* END OF NON-DOCUMENTED BLOCK */
#endif

typedef int (*CZSTREAMWRITER_CALLBACK)(int curr, int max);


/****************************************************************************
  -------------------------------------------------------------------------
 ****************************************************************************/

//! Class to play IT Module files
/*
*/
class ITModule : public ::cz::audio::Module {

	public:

		//! Default constructor
		ITModule(::cz::Core *parentObject);
		//! Destructor
		virtual ~ITModule();

		
		//! Virtual channels.
		/*!
		 * Array of virtual channels used to play the song.
		 * There is no need to mess with this. This is only available to the user with 
		 * the purpose of watching what is being done.
		 */
		VIRTUALCHANNEL *virtualchannels; // Array of virtual channels
		
		//! Tracks of the song
		/*!
		 * Array of tracks of the song.
		 * There is no need to mess with this. This is only available to the user with 
		 * the purpose of watching what is being done.
		 */ 		
		TRACK *track; // pointer to array of size [it_channels]

		
		//! Loads a IT module.
		/*!
		 * This function can be called to reuse the object, instead of creating
		 * a new one every time a new song needs to be played.
		 * 
		 * \param in File to load from
		 * \return ERR_OK on success, other on error.
		 * \sa czFile, Load
		 */
		int Init(::cz::io::File *in);

		static bool CheckFormat(::cz::io::File *in);

        //! Sets mixing parameters
        /*!
         * \param frequency Frequency in hertz.
         *
         *   Don't specify a value greater than the output frequency of the targeted device.
         *  If you do so, you will only be wasting both CPU power and memory.
         *
         * \param stereo True for stereo, False for mono.
         *
         *   Stereo output require twice the CPU power than mono sound. Also, like the frequency
         * parameter, if the device doesn't support stereo, there is no point in mixing in stereo,
         * since the output will be mono anyway.
         *
         * \param bits16 True for 16 bits output, False for 8 bits output
         *
         *   Specifying 16 or 8 bits doesn't affect the performance, so, if the targeted device supports
         * 16 bits output, use 16 bits output.
         *
         * \param numChannels Number of maximum channels to use.
         *
         *   This parameter specifies how many channels should be used to play the song.
         * Specify the lowest possible value. If songs only needs 4 Channels, specify 4.
         *
         * \param bufferMs Size of the buffer used to play the song
         *
         *   This parameter specifies the size in milliseconds of buffer used to play the song.
         * Internally the song uses a looping sound, which keeps getting new sound data, simulating a
         * continuous big sound.
         *   Experience shows you should use a buffer NOT smaller than 500 ms, because the way Mophun's own
         * sound system plays looping sounds. On the other hand, bigger values mean bigger latency.
         *   A good compromise is a value of 1000 ms (1 second), but feel free to experiment with the targeted
         * devices
         *
         * \return ERR_OK on success, other on error.
         */
		//int SetMixingParameters(int frequency, bool stereo, bool bits16, int numChannels, int32_t bufferMs);

		int Start(Mixer *mixer, int firstOrder, int lastOrder, bool loop, u8 volume=20);

		//int GetFirstMixerChannel(void);
		int GetChannels(void);
		int GetPosition(void);
		int GetOrders(void);
		int GetPattern(void);
		int GetPatternRows(void);
		int GetRow(void);
		int GetSpeed(void);
		int SetMasterVolume(u8 vol);

		int Stop();
		int Pause();
		int Resume();
		
		//! Sets the first and last pattern order to play
		/*!
         * \param firstOrder First pattern order to play (0... TotalOrders-1)
         * \param lastOrder Last pattern order to play (firstOrder ... TotalOrders-1, or -1 to play until the end.
         *
         * \note
         *  Leaving the default parameters will play the entire song
		 * 
		 * \return ERR_OK on success, other on error
		 * 
		 * \sa Init, Stop
		 */
		int SetPlayRange(int firstOrder=0, int lastOrder=-1);
				

#if CZ_PLAYER_EXTRAFUNCTIONS_ENABLED


		
		//! Returns the name of the song
		/*! \return The name of the song
		 */
		char *GetModuleName(void);
		
		
		//! Returns the current global volume of the IT.
		/*!
		 * \return The current IT global volume.
		 */				
		int GetGlobalVolume(void);
		

		//! Advance the song to the next pattern while playing
		/*!
		 * \return ERR_OK on success, other on error
		 */
		//int Forward(void);
		
		//! Go backward, and play the previous pattern
		/*!
		 * \return ERR_OK on success, other on error
		 */		
		//int Backward(void);


#endif // CZ_PLAYER_EXTRAFUNCTIONS_ENABLED


protected:
	//virtual int FeedData(void *ptr, int bytes);

private:
		
	u8 Loaded; // is the file loaded ??
	u8 IsPlaying;
	u8 IsPaused;
	u8 PatternEnd;
//	int m_firstMixerChannel;
	
	// Only needed by the Pause function
//	uint8_t Pause_Voices_ON[MAX_VIRTUAL_CHANNELS];
		
	IT_HEADER it_header;
	unsigned char *Orders; // array of orders
	IT_INSTRUMENT *instruments; // array of instruments
	IT_SAMPLE *samples; // Array of samples
	StaticSound **waves; // Array of pointers to czStaticSound Objects
	IT_PATTERN *patterns; // Array of patterns
	
	int dumptr[MAX(MAX(MAX_IT_INSTRUMENTS,MAX_IT_SAMPLES),MAX_IT_PATTERNS)];
	u8 InstrumentMode;
	u8 Linear;
	u8 OldEffects;
	u8 GxxComp;
	u8 it_channels;
	
/****************************************/
	short ProcessRow;
	short NumberOfRows;
	short BreakRow;
	short ProcessOrder;
	short CurrentPattern;
	short CurrentRow;
    
    short m_firstOrder;
    short m_lastOrder;
	
	short TickCounter;
	short Speed;
	short RowCounter;

	short FirstTick;

	int GlobalVol;
	int NextPackPos;
	u8 PatternDelay;
	
	IT_CHANNEL_LAST_INFO *ChLastInfo; // Pointer to array of size [it_channels]
	IT_NOTE *rowinfo; // Pointer to array of size [it_channels]


	u8 m_masterVolume;

/*****************************************/
    virtual void DoTick(void);

	int GetITRow(int patnumber,int pos);
	void MoveToBackGround(TRACK *trk);
	int GetVirtualChannel(int host);
	

	void UpdateEnvelope(IT_ENVELOPE *env, ENVCONTROL *ctrl);
	void PrepareEnvelope(u8 keyon,IT_ENVELOPE *env, ENVCONTROL *ctrl);
    void PrepareEnvelopes(u8 keyon, VIRTUALCHANNEL *vc);
	
	void KillChannel(int ch);
	void DoNoteOff(int channel);


	void DoVolumeSlide(int *Val,unsigned char dat, int upperlimit);
	void DoPitchSlideDown(TRACK *trk,unsigned char dat);
	void DoPitchSlideUp(TRACK *trk,unsigned char dat);
	void DoVibrato(TRACK *trk, unsigned char dat,unsigned char finevibrato);
	void DoTonePortamento(TRACK *trk, unsigned char dat);
	void DoTremor(TRACK *trk, unsigned char dat);
	void DoArpeggio(TRACK *trk, unsigned char dat);
	void DoRetrigg(TRACK *trk, unsigned char dat);
	void DoTremolo(TRACK *trk,unsigned char dat);
	void DoPanbrello(TRACK *trk,unsigned char dat);

	void ProcessEffects(int channel);
	void DoVolEffects(TRACK *trk);
	void DoExtendedEffect(int channel,TRACK *trk,unsigned char nybble1, unsigned char nybble2);

	unsigned int DoSampleVibrato(VIRTUALCHANNEL *vc);
	void PrepareOutput(int ch);

	void DoDCT(int channel);
	void KickNote(int channel);
	void PrepareNote(int channel);
	void CheckChannels(void);

	void CleanUpMemory(void);
	void ResetMembers(void);

	int ReadHeader(::cz::io::File *in);
	int ReadInstruments(::cz::io::File *in);
	int ReadEnvelope(::cz::io::File *in, IT_ENVELOPE *env);
	int ReadSamples(::cz::io::File *in);
	int ReadPatterns(::cz::io::File *in);

	
	// The number of channels used from Device
	//int SoundCardChannelsUsed;
	int m_numVirtualChannels; // maximum channels available for output

};


} // namespace audio

} // namespace cz


#endif
