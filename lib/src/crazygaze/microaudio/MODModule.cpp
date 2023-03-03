//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/MODModule.h>
#include <crazygaze/microaudio/File.h>
#include <crazygaze/microaudio/Mixer.h>
#include <crazygaze/microaudio/StaticSound.h>
#include <crazygaze/microaudio/Memory.h>

#if CZMICROAUDIO_MOD_ENABLED 

namespace cz::microaudio
{

// Default panning to use when loading a MOD.
// I'm not sure about those values, but most players/tracker seem to define their own
const uint8_t g_MOD_defaultPannings[32]={
	32,96,96,32, 32,96,96,32, 32,96,96,32, 32,96,96,32,
	32,96,96,32, 32,96,96,32, 32,96,96,32, 32,96,96,32
};

// Types supported
const char g_MOD_modTypeMK[4] =   {'M', '.', 'K', '.'};
const char g_MOD_modType6CHN[4] = {'6', 'C', 'H', 'N'};
const char g_MOD_modType8CHN[4] = {'8', 'C', 'H', 'N'};

const int16_t g_MOD_periods[MOD_NUMPERIODS] = {
	1814,1801,1788,1775,1762,1749,1737,1724,
	1712,1700,1687,1675,1663,1651,1639,1628, // C-3
	1616,1604,1593,1581,1570,1559,1547,1536, // C#3
	1525,1514,1503,1493,1482,1471,1461,1450, // D-3
	1440,1429,1419,1409,1399,1389,1379,1369, // D#3
	1359,1349,1339,1330,1320,1311,1301,1292, // E-3
	1283,1273,1264,1255,1246,1237,1228,1219, // F-3
	1211,1202,1193,1185,1176,1168,1159,1151, // F#3
	1143,1134,1126,1118,1110,1102,1094,1086, // G-3
	1078,1071,1063,1055,1048,1040,1033,1025, // G#3
	1018,1011,1003, 996, 989, 982, 975, 968, // A-3
	961, 954, 947, 940, 933, 927, 920, 913,  // A#3
	907, 900, 894, 887, 881, 875, 868, 862,  // B-3
	856, 850, 844, 838, 832, 826, 820, 814,  // C-4
	808, 802, 796, 791, 785, 779, 774, 768,  // C#4
	763, 757, 752, 746, 741, 736, 730, 725,  // D-4
	720, 715, 709, 704, 699, 694, 689, 684,  // D#4
	679, 675, 670, 665, 660, 655, 651, 646,  // E-4
	641, 637, 632, 628, 623, 619, 614, 610,  // F-4
	605, 601, 597, 592, 588, 584, 580, 575,  // F#4
	571, 567, 563, 559, 555, 551, 547, 543,  // G-4
	539, 535, 532, 528, 524, 520, 516, 513,  // G#4
	509, 505, 502, 498, 494, 491, 487, 484,  // A-4
	480, 477, 474, 470, 467, 463, 460, 457,  // A#4
	453, 450, 447, 444, 441, 437, 434, 431,  // B-4
	428, 425, 422, 419, 416, 413, 410, 407,  // C-5
	404, 401, 398, 395, 392, 390, 387, 384,  // C#5
	381, 379, 376, 373, 370, 368, 365, 363,  // D-5
	360, 357, 355, 352, 350, 347, 345, 342,  // D#5
	340, 337, 335, 332, 330, 328, 325, 323,  // E-5
	321, 318, 316, 314, 312, 309, 307, 305,  // F-5
	303, 300, 298, 296, 294, 292, 290, 288,  // F#5
	286, 284, 282, 280, 278, 276, 274, 272,  // G-5
	270, 268, 266, 264, 262, 260, 258, 256,  // G#5
	254, 253, 251, 249, 247, 245, 244, 242,  // A-5
	240, 238, 237, 235, 233, 232, 230, 228,  // A#5
	227, 225, 223, 222, 220, 219, 217, 216,  // B-5
	214, 212, 211, 209, 208, 206, 205, 203,  // C-6
	202, 201, 199, 198, 196, 195, 193, 192,  // C#6
	191, 189, 188, 187, 185, 184, 183, 181,  // D-6
	180, 179, 177, 176, 175, 174, 172, 171,  // D#6
	170, 169, 167, 166, 165, 164, 163, 161,  // E-6
	160, 159, 158, 157, 156, 155, 154, 152,  // F-6
	151, 150, 149, 148, 147, 146, 145, 144,  // F#6
	143, 142, 141, 140, 139, 138, 137, 136,  // G-6
	135, 134, 133, 132, 131, 130, 129, 128,  // G#6
	127, 126, 125, 125, 124, 123, 122, 121,  // A-6
	120, 119, 118, 118, 117, 116, 115, 114,  // A#6
	113, 113, 112, 111, 110, 109, 109, 108,  // B-6
	107, 106, 105, 105, 104, 103, 102, 102,  // C-7
	101, 100, 100,  99,  98,  97,  97,  96,  // C-7
	95,  95,  94,  93,  93,  92,  91,  91,   // D-7
	90,  89,  89,  88,  87,  87,  86,  86,   // D-7
	85,  84,  84,  83,  83,  82,  81,  81,   // E-7
	80,  80,  79,  78,  78,  77,  77,  76,   // F-7
	76,  75,  75,  74,  74,  73,  72,  72,   // F-7
	71,  71,  70,  70,  69,  69,  68,  68,   // G-7
	67,  67,  66,  66,  65,  65,  65,  64,   // G-7
	64,  63,  63,  62,  62,  61,  61,  60,   // A-7
	60,  60,  59,  59,  58,  58,  58,  57,   // A-7
	57,  56,  56,  55,  55,  55,  54,  54    // B-7
};

const int8_t g_MOD_sineWave[MOD_NUMWAVEFORMINDEXES] =
{
	0,12,25,37,49,60,71,81,90,98,106,112,117,122,125,126,
	127,126,125,122,117,112,106,98,90,81,71,60,49,37,25,12,
	0,-12,-25,-37,-49,-60,-71,-81,-90,-98,-106,-112,-117,-122,-125,-126,
	-127,-126,-125,-122,-117,-112,-106,-98,-90,-81,-71,-60,-49,-37,-25,-12
};

const int8_t g_MOD_rampdownWave[MOD_NUMWAVEFORMINDEXES] =
{
	0,-4,-8,-12,-16,-20,-24,-28,-32,-36,-40,-44,-48,-52,-56,-60,
	-64,-68,-72,-76,-80,-84,-88,-92,-96,-100,-104,-108,-112,-116,-120,-124,
	127,123,119,115,111,107,103,99,95,91,87,83,79,75,71,67,
	63,59,55,51,47,43,39,35,31,27,23,19,15,11,7,3
};

const int8_t g_MOD_squareWave[MOD_NUMWAVEFORMINDEXES] =
{
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
	-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
	-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127
};

const int8_t g_MOD_randomWave[MOD_NUMWAVEFORMINDEXES] =
{
	98,-127,-43,88,102,41,-65,-94,125,20,-71,-86,-70,-32,-16,-96,
	17,72,107,-5,116,-69,-62,-40,10,-61,65,109,-18,-38,-13,-76,
	-23,88,21,-94,8,106,21,-112,6,109,20,-88,-30,9,-127,118,
	42,-34,89,-4,-51,-72,21,-29,112,123,84,-101,-92,98,-54,-95
};

const int8_t* g_MOD_waveFormsPtrs[4] = {
	g_MOD_sineWave,
	g_MOD_rampdownWave,
	g_MOD_squareWave,
	g_MOD_randomWave
};



#define CLEARARRAY(a,n) memset(a, 0, sizeof(a[0])*(n))

inline int Clamp(int val, int minval, int maxval) 
{
	if (val<minval) return minval;
	else if (val>maxval) return maxval;
	else return val;
}


enum Effect{
	EfArpeggio = 0,
	EfSlideUp = 1,
	EfSlideDown = 2,
	EfTonePortamento = 3,
	EfVibrato = 4,
	EfTonePortamentoAndVolumeSlide = 5,
	EfVibratoAndVolumeSlide = 6,
	EfTremolo = 7,
	EfSetPanPosition = 8,
	EfSetSampleOffset = 9,
	EfVolumeSlide = 0xA,
	EfPositionJump = 0xB,
	EfSetVolume = 0xC,
	EfPatternBreak = 0xD,
	EfExtendedEffect = 0xE,
	EfSetSpeed = 0xF
};

enum ExtendedEffect {
	EfExtSetFilter = 0,
	EfExtFinePortaUp = 1,
	EfExtFinePortaDown = 2,
	EfExtGlissandoControl = 3,
	EfExtSetVibratoWaveform = 4,
	EfExtSetFinetune = 5,
	EfExtPatternLoop = 6,
	EfExtSetTremoloWaveform = 7,
	EfExtPanning = 8,
	EfExtRetrigNote = 9,
	EfExtFineVolSlideUp = 0xA,
	EfExtFineVolSlideDown = 0xB,
	EfExtCutNote = 0xC,
	EfExtDelayNote = 0xD,
	EfExtPatternDelay = 0xE,
	EfExtInvertLoop = 0xF
};


MODModule::MODModule()
{
	ResetMembers();
}

MODModule::~MODModule()
{
	CleanUpMemory();
}

void MODModule::CleanUpMemory(void)
{
	if (m_tempPatternData!=NULL) CZMICROAUDIO_FREE(m_tempPatternData);
	if (m_packedPatternData!=NULL) CZMICROAUDIO_FREE(m_packedPatternData);
	for (int numSample = 0; numSample<MOD_NUMSAMPLES; numSample++){
		MODSAMPLE *smp = &m_samples[numSample];
		if (smp->snd!=NULL){
			CZMICROAUDIO_DELETE(smp->snd);
			smp->snd = NULL;
		}
	}
	if (m_tracks!=NULL) CZMICROAUDIO_FREE(m_tracks);
}

void MODModule::ResetMembers(void)
{
	m_tempPatternData = NULL;
	m_packedPatternData = NULL;
	m_tracks = NULL;
	CLEARARRAY(m_samples, MOD_NUMSAMPLES);
	CLEARARRAY(m_orders, MOD_NUMORDERS);
	m_isPlaying = false;
	m_loaded = false;
	m_isPaused = false;
	m_bpm = 125;
	m_speed = 6;
	m_firstMixerChannel=0;
	m_mixer = NULL;
}

inline uint16_t ReadBigEndianWord(File *in) {
	return (uint16_t)(((uint16_t)in->ReadUnsigned8() << 8) + (uint16_t)in->ReadUnsigned8());
}


bool MODModule::CheckFormat(File *in)
{

	// Check the MOD type
	int pos = in->GetPos();
//	if (in->GetSize()<(1080+4)) return false;
	char buf[4];
	in->Seek(1080, FILE_SEEK_START);
	in->ReadData(buf, 4);
	in->Seek(pos, FILE_SEEK_START);
	if ((memcmp(buf, g_MOD_modTypeMK, 4)==0)||
		(memcmp(buf, g_MOD_modType6CHN, 4)==0)||
		(memcmp(buf, g_MOD_modType8CHN, 4)==0)) {
		return true;
	} else {
		return false;
	}
}

int MODModule::Init(File *in)
{
	int numSample;

	if (m_isPlaying) CZERROR(Error::CantRun);

	CleanUpMemory();
	ResetMembers();

	// Check the MOD type
//	if (in->GetSize()<(1080+4)) CZERROR(Error::WrongFormat);
	char buf[4];
	in->Seek(1080, FILE_SEEK_START);
	in->ReadData(buf, 4);
	if (memcmp(buf, g_MOD_modTypeMK, 4)==0){
		m_numChannels = 4;
		CZMICROAUDIO_LOG(LogLevel::Log, "MOD of type M.K. (4 channels)\n");
	} else if (memcmp(buf, g_MOD_modType6CHN, 4)==0) {
		m_numChannels = 6;
		CZMICROAUDIO_LOG(LogLevel::Log, "MOD of type 6CHN\n");
	} else if (memcmp(buf, g_MOD_modType8CHN, 4)==0) {
		m_numChannels = 8;
		CZMICROAUDIO_LOG(LogLevel::Log, "MOD of type 8CHN\n");
	} else CZERROR(Error::WrongFormat);

	// Allocate the memory needed for tracks
	m_tracks = (MODTRACK*) CZMICROAUDIO_ALLOC(sizeof(MODTRACK)*m_numChannels);
	if (m_tracks==NULL) CZERROR(Error::OutOfMemory);
	CLEARARRAY(m_tracks, m_numChannels);

	// Read song name (20 bytes)
	in->Seek(0, FILE_SEEK_START);
	in->ReadData(m_songName, MOD_SONGNAMELENGTH);
	m_songName[MOD_SONGNAMELENGTH-1] = 0; // Set last char to NULL


	// Read Samples
	in->Seek(20, FILE_SEEK_START);
	for (numSample = 0; numSample < MOD_NUMSAMPLES; numSample++){
		MODSAMPLE *smp = &m_samples[numSample];
		in->ReadData(smp->name, MOD_SAMPLENAMELENGTH);
		smp->name[MOD_SAMPLENAMELENGTH-1] = 0;
		smp->length = (int)ReadBigEndianWord(in) * 2;
		smp->fineTune = in->ReadSigned8();
		if (smp->fineTune > 7) smp->fineTune -= 16;
		smp->volume = in->ReadUnsigned8();
		smp->loopStart = (int)ReadBigEndianWord(in) * 2;
		smp->loopLength = (int)ReadBigEndianWord(in) * 2;
		CZMICROAUDIO_LOG(LogLevel::Log, "SAMPLE %d - Name=%s,  length=%d, fineTune=%d, volume=%d, loopStart=%d, loopLength=%d\n", numSample
			, smp->name, (int)smp->length, (int)smp->fineTune, (int)smp->volume, (int)smp->loopStart, (int)smp->loopLength);
	}

	// number of orders (1 byte)
	m_numOrders = in->ReadUnsigned8();
	CZMICROAUDIO_LOG(LogLevel::Log, "Num of orders = %d\n", (int)m_numOrders);
	// discard unused byte ( song jump position???? )
	in->ReadUnsigned8();
	// Read pattern order, and calculate the number of patterns
	m_numPatterns = 0;
	in->ReadData(m_orders, MOD_NUMORDERS);
	for (int i=0; i<MOD_NUMORDERS; i++) if (m_orders[i]>m_numPatterns) m_numPatterns = m_orders[i];
	m_numPatterns++;
	CZMICROAUDIO_LOG(LogLevel::Log, "Num of patterns = %d\n", (int)m_numPatterns);

	// Discard 4 bytes (we are at the tag position again)
	in->ReadUnsigned32();

	// Read pattern data
	// Allocate memory for all our pattern data structure
	m_packedPatternData = (MODPACKEDNOTE*) CZMICROAUDIO_ALLOC(sizeof(MODPACKEDNOTE)*m_numChannels*64*m_numPatterns);
	if (m_packedPatternData==NULL) CZERROR(Error::OutOfMemory);

	// Allocate memory for only one pattern to read from file
	int filePatternSize = m_numChannels*4*64;
	m_tempPatternData = (uint8_t*) CZMICROAUDIO_ALLOC(sizeof(uint8_t)*filePatternSize);
	if (m_tempPatternData==NULL) CZERROR(Error::OutOfMemory);

	// Read one pattern at a time, and pack the pattern data to the new structure, take takes only 3 bytes per note.
	for (int pat=0; pat<m_numPatterns; pat++)
	{
		in->ReadData(m_tempPatternData, filePatternSize);

		uint8_t *ptr1 = m_tempPatternData;
		MODPACKEDNOTE *ptr2 = &m_packedPatternData[m_numChannels*64*pat];
		int i= m_numChannels*64;
		while(i--){
			int period;
			uint8_t sample,effect, effectParam;
			sample = (ptr1[0] & 0xF0) + (ptr1[2]>>4);
			effect = ptr1[2] & 0x0F;
			effectParam = ptr1[3];
			period = (int(ptr1[0] & 0x0F) << 8) + ptr1[1];

			// Convert period to note (0..60) -Later, multiply by 8 to get the real index into the period table
			uint8_t note = 127; // Default to 127 (all 7 bits on), so that we can detect later if there is a note
			if (period>0)
			{
				for (int n=1; n<=60; n++)
				{
					// Accept the note with the nearest period
					if (INBETWEEN(period, g_MOD_periods[8*n + 4], g_MOD_periods[8*n - 4] ))
					{
						note = n;
						break;
					}
				}

				if (note==127)
				{
					// If the note is out of range, also clear the sample
					sample = 0;
					CZMICROAUDIO_LOG(LogLevel::Warning,
						"Note period out of supported range (%d), in pattern %d, row %d, channel %d \n",
						period,
						pat,
						(m_numChannels*64-(i+1)) / m_numChannels,
						(m_numChannels*64-(i+1)) % m_numChannels +1);
				}

			}
			
			// Now that we have all the necessary data, pack it into our own structure
			// , that only takes 3 bytes, instead of 4.
			ptr2->b0 = (note<<1) |((sample>>4)&(0x01));
			ptr2->b1 = ((sample&0x0F)<<4) | (effect&0x0F);
			ptr2->b2 = effectParam;
			
			ptr1+=4;
			ptr2++;
		}

	}
	// Free the temporary buffer
	CZMICROAUDIO_FREE(m_tempPatternData);
	m_tempPatternData = NULL;

	// Read sample data
	for (numSample = 0; numSample<MOD_NUMSAMPLES; numSample++){
		MODSAMPLE *smp = &m_samples[numSample];
		if (smp->length>0) {
			smp->snd = CZMICROAUDIO_NEW(StaticSound);
			if (smp->snd==NULL) CZERROR(Error::OutOfMemory);
			int err= smp->snd->Set(SOUND_8BITS|SOUND_MONO|SOUND_SIGNED|SOUND_LOOP_OFF, smp->length);
			if (err!=Error::Success) CZERROR(static_cast<Error>(err));
			in->ReadData(smp->snd->GetPtr(), smp->length);

			smp->snd->SetLoopMode((smp->loopLength>2) ? SOUND_LOOP_NORMAL : SOUND_LOOP_OFF, smp->loopStart, smp->loopLength);
			smp->snd->SmoothLoop();

			/*
			czDiskFile out;
			out.Open("sample.raw",0,CZFILE_WRITE);
			out.WriteData((uint8_t*)smp->snd->GetPtr()-SAFETYAREA, smp->length+(SAFETYAREA*2));
			out.Close();
			*/


			CZMICROAUDIO_LOG(LogLevel::Log, "SAMPLE %d - %d bytes\n", numSample+1, smp->length);
		}
	}

	// Make sure no bytes are left at the end of the file
/*	int left = in->GetSize() - in->GetPos();
	if (left!=0) {
		CZMICROAUDIO_LOG(LogLevel::Warning, "%d Bytes left at the end of the file. Possible invalid MOD or loading BUG.", left);
		CZERROR(Error::WrongFormat);
	}*/

	m_loaded = true;

	return Error::Success;
}

int MODModule::Start(Mixer *mixer, int firstOrder, int lastOrder, bool loop, uint8_t volume)
{
	if ((!m_loaded)||(m_isPlaying)) CZERROR(Error::CantRun);

	// Reserve the channels from the mixer
	int fch = mixer->ReserveChannels(m_numChannels);
	if (fch<0){
		CZMICROAUDIO_LOG(LogLevel::Warning, "Not enough consecutive channels to play song. Requires %d channels\n", (int)m_numChannels);
		CZERROR(static_cast<Error>(fch));
	}
	m_firstMixerChannel = fch;

	// clean track information
	CLEARARRAY(m_tracks, m_numChannels);

	// Set some default values
	for (int ch=0; ch<m_numChannels; ch++){
		MODTRACK *trk = &m_tracks[ch];
		trk->mixerChannelUsed = m_firstMixerChannel+ch;
		trk->vibratoRetrig = true;
		trk->tremoloRetrig = true;
		// Set default panning
		trk->panning = g_MOD_defaultPannings[ch];
	}

	m_bpm = 125;
	m_speed = 6;
	m_firstOrder = CLAMP(firstOrder, 0, m_numOrders-1);
	m_lastOrder = (lastOrder==-1) ? m_numOrders-1 : CLAMP(lastOrder, m_firstOrder, m_numOrders-1);
	m_currentOrder = m_firstOrder;
	m_currentTick = m_speed;
	m_jumpToOrder = -1;
	m_patternBreakToRow = -1;
	m_currentRow = -1; // Start at -1, because in the first loop, it will be incremented from -1 to 0
	m_patternLoopRow = 0;
	m_patternLoopCount = 0;
	m_patternLoopTo = -1;
	m_patternDelay = 0;
	m_reachedEnd = false;
	m_isPaused = false;

	m_mixer = mixer;
	m_mixer->SetMasterVolume(volume, m_firstMixerChannel, m_numChannels);
	m_isPlaying = true;
	m_loop = loop;
	return Error::Success;
}

int MODModule::Stop(void)
{
	if ((!m_loaded)||(!m_isPlaying)) return Error::Success;	
	for (int ch=m_firstMixerChannel; ch<m_firstMixerChannel+m_numChannels; ch++) m_mixer->SetVoiceStatus(ch, false);
	m_mixer->FreeChannels(m_firstMixerChannel, m_numChannels);
	m_isPlaying = false;
	m_mixer = NULL;
	return Error::Success;
}

int MODModule::Pause(void)
{
	if ((m_isPaused)||(!m_loaded)||(!m_isPlaying)) return Error::Success;	
	for(int t=0; t<m_numChannels; t++){
		if (m_mixer->IsVoiceON(m_tracks[t].mixerChannelUsed)){
			m_mixer->SetVoiceStatus(m_tracks[t].mixerChannelUsed, false);
			m_tracks[t].wasOnBeforePause = true;
		} else {
			m_tracks[t].wasOnBeforePause = false;
		}
	}
	m_isPaused = true;
	return Error::Success;
}

int MODModule::SetMasterVolume(uint8_t vol){
	m_mixer->SetMasterVolume(vol, m_firstMixerChannel, m_numChannels);
	return Error::Success;
}

int MODModule::Resume(void){
	if (!m_isPaused) return Error::Success;	
	for(int t=0; t<m_numChannels; t++){
		if (m_tracks[t].wasOnBeforePause) m_mixer->SetVoiceStatus(m_tracks[t].mixerChannelUsed, true);
	}
	m_isPaused=false;	
	return Error::Success;
}

void MODModule::DecodeRow(void)
{
	MODPACKEDNOTE *ptr = &m_packedPatternData[(m_numChannels*64*m_orders[m_currentOrder]) + (m_numChannels*m_currentRow)];
	for (int ch = 0; ch < m_numChannels; ch++){
		MODNOTE *note = &m_tracks[ch].note;
		// Read note, and convert to real index
		note->periodIndex = ((int16_t)(ptr->b0 >>1));
		note->periodIndex = (note->periodIndex==127) ? -1 : (note->periodIndex*8);
		note->sample = ((ptr->b0 & 0x01)<<4) | ((ptr->b1 >> 4)&0x0F);
		note->effect = ptr->b1 & 0x0F;
		note->effectParam = ptr->b2;
		ptr++;
	}
}


int MODModule::CalculatePeriod(MODTRACK *trk, int periodIndex, int periodIncrement)
{
	int finetune = 0;
	if (trk->samplePtr!=NULL) finetune = trk->samplePtr->fineTune;
	return g_MOD_periods[Clamp(periodIndex+finetune, (int)0, (int)MOD_NUMPERIODS-1)] + periodIncrement;
}

int MODModule::CalculateGlissandoSemitone(MODTRACK *trk, int period)
{
	int finetune = 0;
	if (trk->samplePtr!=NULL) finetune = trk->samplePtr->fineTune;
	if (trk->periodDirection<0){
		int index = finetune;
		while (g_MOD_periods[index+8]>=period) index +=8;
		return g_MOD_periods[index];
	} else if (trk->periodDirection>0){
		int index = (MOD_NUMPERIODS - 8) +finetune;
		while (g_MOD_periods[index-8]<=period) index -=8;
		return g_MOD_periods[index];
	} else return period;
}


void MODModule::ProcessRow(void)
{
	MODTRACK *trk;
	MODNOTE *note;
	for (int ch = 0; ch < m_numChannels; ch++){
		trk = &m_tracks[ch];
		note = &m_tracks[ch].note;
		
		if (note->sample>0) {
			trk->sampleNum = note->sample;
			trk->samplePtr = &m_samples[note->sample-1];
			if (trk->samplePtr->snd!=NULL){
				trk->currentVolume = trk->samplePtr->volume; // Set channel volume to the sample default volume
				// Set the sample in the mixer
				trk->flagSetSample = true;
			}
		}

		// Update the frequency when there is period, but NOT a tone portamento, because when there is a tone
		// portamento effect, we should keep the old frequency
		if ( (note->periodIndex>=0) && (note->effect!=EfTonePortamento) && (note->effect!=EfTonePortamentoAndVolumeSlide)) {
			if (! ((note->effect==EfExtendedEffect)&&((note->effectParam&0xF0)==0xD)) ){
				if (trk->samplePtr!=NULL) {
					if (trk->samplePtr->snd!=NULL){
						trk->flagResetSample = true;
					}
				}
				trk->originalPeriodIndex = note->periodIndex;
				trk->currentPeriod = CalculatePeriod(trk, trk->originalPeriodIndex);

				// reset wave position for vibrato if necessary
				if (trk->vibratoRetrig) trk->vibratoWavePos = 0;
				// reset wave position for tremolo if necessary
				if (trk->tremoloRetrig) trk->tremoloWavePos = 0;

			}
		}

	}
}




void MODModule::DoVibrato(MODTRACK *trk, bool vibratoAndVolSlide)
{

	if (m_currentTick==0){
		if (vibratoAndVolSlide==false) {
			// Memorize parameters
			uint8_t x = trk->note.effectParam >> 4;
			uint8_t y = trk->note.effectParam & 0x0F;
			if (x!=0) trk->vibratoSpeed = x;
			if (y!=0) trk->vibratoDepth = y;
		}
	}

	// delta = (depth*val) / 64
	const int8_t *table = g_MOD_waveFormsPtrs[trk->vibratoWaveform];
	int delta = ((int)trk->vibratoDepth * (int)table[trk->vibratoWavePos]) >> 6;
	// Add or subtract around the current period
	trk->periodToPlay = trk->currentPeriod + delta;

	// Update wave position, and wrap around if necessary
	if (m_currentTick>0){
		trk->vibratoWavePos += trk->vibratoSpeed;
		if (trk->vibratoWavePos>=MOD_NUMWAVEFORMINDEXES){
			trk->vibratoWavePos -= MOD_NUMWAVEFORMINDEXES;
		}
	}
}

void MODModule::DoTremolo(MODTRACK *trk)
{

	if (m_currentTick==0){
		if (trk->note.periodIndex>=0) {
			// if there is a note indication, then reset wave position
			trk->tremoloWavePos = 0;
		}
		// Memorize parameters
		uint8_t x = trk->note.effectParam >> 4;
		uint8_t y = trk->note.effectParam & 0x0F;
		if (x!=0) trk->tremoloSpeed = x;
		if (y!=0) trk->tremoloDepth = y;
	}


	// delta = (depth*val) / 32
	const int8_t *table = g_MOD_waveFormsPtrs[trk->tremoloWaveform];
	int delta = ((int)trk->tremoloDepth * (int)table[trk->tremoloWavePos]) >> 5;
	// Add or subtract around the current volume
	trk->volumeToPlay = (uint8_t) Clamp((int)trk->currentVolume + delta, (int)0, (int)64);

	// Update wave position, and warp around if necessary
	if (m_currentTick>0){
		trk->tremoloWavePos += trk->tremoloSpeed;
		if (trk->tremoloWavePos>=MOD_NUMWAVEFORMINDEXES){
			trk->tremoloWavePos -= MOD_NUMWAVEFORMINDEXES;
		}
	}
}

void MODModule::DoTonePortamento(MODTRACK *trk, bool tonePortamentoAndVolSlide)
{
	if (m_currentTick==0){
		if (trk->note.periodIndex>=0){
			trk->portamentoTargetPeriod = CalculatePeriod(trk,trk->note.periodIndex);
			if (trk->portamentoTargetPeriod > trk->currentPeriod) trk->periodDirection = 1;
			else if (trk->portamentoTargetPeriod < trk->currentPeriod) trk->periodDirection = -1;
			else trk->periodDirection = 0;
		}
		if ((tonePortamentoAndVolSlide==false)&&(trk->note.effectParam!=0)) trk->portamentoSpeed = trk->note.effectParam;
	} else {
		trk->currentPeriod += trk->portamentoSpeed*trk->periodDirection;
		// test if target was reached, and if so, make sure it doesn't pass over it
		if (((trk->periodDirection>0)&&(trk->currentPeriod > trk->portamentoTargetPeriod)) ||
			((trk->periodDirection<0)&&(trk->currentPeriod < trk->portamentoTargetPeriod)) ) trk->currentPeriod = trk->portamentoTargetPeriod;
	}

	if (!trk->glissando){
		trk->periodToPlay = trk->currentPeriod;
	} else {
		// Find the semitone to use
		trk->periodToPlay = CalculateGlissandoSemitone(trk, trk->currentPeriod);
	}

}

void MODModule::DoVolumeSlide(MODTRACK *trk)
{
	if (m_currentTick>0){
		uint8_t x = trk->note.effectParam >> 4;
		uint8_t y = trk->note.effectParam & 0x0F;
		int tmpvol = trk->currentVolume;
		// if both x and y are zero, then do nothing
		if ((x>0)&&(y==0)){
			tmpvol += x;
		} else if ((y>0)&&(x==0)){
			tmpvol -= y;
		}
		trk->currentVolume = (uint8_t) Clamp(tmpvol, (int)0, (int)64);
		trk->volumeToPlay = trk->currentVolume;
	}
}

void MODModule::DoPortamentoUp(MODTRACK *trk, bool fine)
{
	if (fine) {
		if (m_currentTick==0){
			trk->currentPeriod -= trk->note.effectParam & 0x0F;
			trk->periodToPlay = trk->currentPeriod;
		}
	} else if (m_currentTick>0){
		trk->currentPeriod -= trk->note.effectParam;
		trk->periodToPlay = trk->currentPeriod;
	}
}

void MODModule::DoPortamentoDown(MODTRACK *trk, bool fine)
{
	if (fine) {
		if (m_currentTick==0){
			trk->currentPeriod += trk->note.effectParam & 0x0F;
			trk->periodToPlay = trk->currentPeriod;
		}
	} else if (m_currentTick>0){
		trk->currentPeriod += trk->note.effectParam;
		trk->periodToPlay = trk->currentPeriod;
	}
}


void MODModule::ProcessFirstTickEffects(void)
{
	for (int ch=0; ch<m_numChannels; ch++){
		MODTRACK *trk = &m_tracks[ch];
		uint8_t effect = trk->note.effect;
		uint8_t effectParam = trk->note.effectParam;
		// skip processing if no effect
		if ((effect==0)&&(effectParam==0)) continue;

		uint8_t effectX = (effectParam&0xF0) >> 4;
		uint8_t effectY = effectParam&0x0F;

		switch (effect) {
			case EfSetPanPosition : // 8xy
				// Surround (0xA4) not supported
				if (effectParam!=0xA4){
					trk->panning = effectParam;
				}
			break;
			case EfSetSampleOffset : // 9xy
				if (trk->note.effectParam!=0) trk->sampleOffset = trk->note.effectParam;
			break;
			case EfPositionJump : // Bxy
				m_jumpToOrder = (effectParam<m_numOrders) ? effectParam : m_numOrders-1;
			break;
			case EfSetVolume : // effect Cxy
				trk->currentVolume = MIN(0x40, effectParam);
				break;
			case EfPatternBreak : // effect Dxy
				{
					uint8_t row = effectX*10 + effectY; // represented as decimal
					m_patternBreakToRow = (row<=63) ? row : 0;
				}
			break;
			case EfSetSpeed : // effect Fxy
				// If parameter is in range 0x00-0x1F, then is a set speed command, or else if a set tempo command
				if (effectParam <= 0x1F ) m_speed = effectParam; else m_bpm = effectParam;
			break;


			// Extended Effects
			case EfExtendedEffect : // Ey

				switch (effectX){

					case EfExtSetFilter : // E0y  - Not supported
					break;
					case EfExtFinePortaUp : // E1y
						DoPortamentoUp(trk, true);
					break;
					case EfExtFinePortaDown : // E2y
						DoPortamentoDown(trk, true);
					break;
					case EfExtGlissandoControl : // E3y
						if (effectY==1) trk->glissando=true;
						else if (effectY==0) trk->glissando=false;
					break;
					case EfExtSetVibratoWaveform : // E4y
						if (effectY<=3){ // for values 0..3
							trk->vibratoWaveform = effectY;
							trk->vibratoRetrig = true;
						} else if (effectY<=7){ // for values 4..7
							trk->vibratoWaveform = effectY-4;
							trk->vibratoRetrig = false;
						}
					break;
					case EfExtSetFinetune : // E5y
						if (trk->note.sample>0){
							m_samples[trk->note.sample-1].fineTune = (effectY>7) ? (int8_t)effectY-16 : (int8_t)effectY;
						}
					break;
					case EfExtPatternLoop : // E6y
						if (effectY==0){
							m_patternLoopRow = m_currentRow;
						} else {
							if (m_patternLoopCount==0) m_patternLoopCount = effectY;
							else m_patternLoopCount--;
							if (m_patternLoopCount>0) m_patternLoopTo = m_patternLoopRow;
						}
					break;
					case EfExtSetTremoloWaveform : // E7y
						if (effectY<=3){ // for values 0..3
							trk->tremoloWaveform = effectY;
							trk->tremoloRetrig = true;
						} else if (effectY<=7){ // for values 4..7
							trk->tremoloWaveform = effectY-4;
							trk->tremoloRetrig = false;
						}
					break;
					case EfExtPanning : // E8y
						trk->panning = effectY*8;
					break;
					case EfExtFineVolSlideUp : // EAy
						trk->currentVolume = Clamp(trk->currentVolume+effectY, 0, 64);
						trk->volumeToPlay = trk->currentVolume;
					break;
					case EfExtFineVolSlideDown : // EBy
						trk->currentVolume = (uint8_t) Clamp((int)trk->currentVolume-(int)effectY, (int)0, (int)64);
						trk->volumeToPlay = trk->currentVolume;
					break;
					case EfExtPatternDelay : // EEy
						m_patternDelay = (int)effectY * m_speed;
					break;
				}

			break;

		};
		
	}
}



void MODModule::UpdateEffects(void)
{
	for (int ch=0; ch<m_numChannels; ch++){
		MODTRACK *trk = &m_tracks[ch];
		uint8_t effect = trk->note.effect;
		uint8_t effectParam = trk->note.effectParam;

		uint8_t effectX = (effectParam&0xF0) >> 4;
		uint8_t effectY = effectParam&0x0F;


		trk->periodToPlay = trk->currentPeriod;
		trk->volumeToPlay = trk->currentVolume;

		// skip processing if no effect
		if ((effect==0)&&(effectParam==0)) continue;


		switch (effect){
			case EfArpeggio: // 0xy
				if (m_currentTick==0) if (effectParam!=0) trk->arpeggioCounter = 0;
				switch (trk->arpeggioCounter) {
					case 0: trk->periodToPlay = CalculatePeriod(trk, trk->originalPeriodIndex); break; // set period of original note
					case 1: trk->periodToPlay = CalculatePeriod(trk, trk->originalPeriodIndex+(effectParam >> 4)*8); break; // Add x semitones
					case 2:
						trk->periodToPlay = CalculatePeriod(trk, trk->originalPeriodIndex+(effectParam&0x0F)*8);  // Add y semitones
						trk->arpeggioCounter = -1; // -1 because it will be incremented to 0 (zero) just below this
						break;
				}
				trk->arpeggioCounter++;
			break;

			case EfSlideUp: // 1xy
				DoPortamentoUp(trk);
			break;

			case EfSlideDown: // 2xy
				DoPortamentoDown(trk);
			break;

			case EfTonePortamento: // 3xy
				DoTonePortamento(trk, false);
			break;

			case EfVibrato: // 4xy
				DoVibrato(trk, false);
			break;

			case EfTonePortamentoAndVolumeSlide : // 5xy
				DoTonePortamento(trk, true);
				DoVolumeSlide(trk);
			break;

			case EfVibratoAndVolumeSlide : // 6xy
				DoVibrato(trk, true);
				DoVolumeSlide(trk);
			break;

			case EfTremolo: // 7xy
				DoTremolo(trk);
			break;

			case EfVolumeSlide : // Axy
				DoVolumeSlide(trk);
			break;


			case EfExtendedEffect : // Ey
				
				switch(effectX){

					case EfExtRetrigNote : // E9y
						if (m_currentTick>0){
							if ((m_currentTick % effectY)==0){
								trk->flagResetSample = true;
							}
						}
					break;
					case EfExtCutNote : // ECy
						if (m_currentTick==effectY) {
							trk->currentVolume = 0;
							trk->volumeToPlay = 0;
						}
					break;
					case EfExtDelayNote : // EDy
						// Only process if there is a note indication
						if (trk->note.periodIndex>=0) {
							if (m_currentTick==0){ // Avoid playing the note at this tick
								trk->flagResetSample = false;
								trk->flagSetSample = false;
							}
							if (m_currentTick==effectY){
								trk->flagResetSample = true;
								trk->flagSetSample = true;
							}
						}
					break;
				}

			break;

		}

	}
}

inline int PeriodToFrequency(int period){
	return (7159090 / (period<<1));
}

void MODModule::UpdateMixer(void)
{
	for (int ch=0; ch<m_numChannels; ch++){
		MODTRACK *trk = &m_tracks[ch];
		int mxch = trk->mixerChannelUsed;

		if (trk->flagSetSample){
			int end;
			int loopmode;
			if (trk->samplePtr->loopLength>2){
				end = trk->samplePtr->loopStart + trk->samplePtr->loopLength;
				loopmode = SOUND_LOOP_NORMAL;
			} else {
				end = trk->samplePtr->length;
				loopmode = SOUND_LOOP_OFF;
			}
			m_mixer->SetSample(mxch, trk->samplePtr->snd, 0, end , trk->samplePtr->loopStart, loopmode);
			trk->flagSetSample = false;
			m_mixer->SetVoiceStatus(mxch, true);
		}
		if (trk->flagResetSample){
			int sampleOffset = 0;
			// Process effect 9xy (set sample offset)
			if (trk->note.effect==EfSetSampleOffset){
				sampleOffset = (int)trk->sampleOffset*0x100;
				if (sampleOffset > trk->samplePtr->length) sampleOffset = 0;
			}
			m_mixer->SetPosition(mxch, sampleOffset);
			m_mixer->SetVoiceStatus(mxch, true); // turn channel on, because it may be already off.
			trk->flagResetSample = false;
		}

		if (trk->sampleNum>0){
			m_mixer->SetFrequency(mxch, PeriodToFrequency(trk->periodToPlay));
			int vol = trk->volumeToPlay*4;
			m_mixer->SetVolume(mxch, MIN(255,vol));
		}


		int pan = (uint8_t) Clamp((int)trk->panning*2, (int)0, (int)255);
		m_mixer->SetPanning(trk->mixerChannelUsed, (uint8_t) pan);
	}
}

void MODModule::DoTick(void)
{

	if ((m_reachedEnd==true)||(m_loaded==false)||(m_isPlaying==false)) return;

	m_currentTick++;

	if (m_currentTick >= m_speed) {
		
		if (m_patternDelay==0){

			// Process jump to pattern if necessary, or pattern break
			if (m_jumpToOrder>=0){
				m_currentRow = -1; // -1, because it will be incremented to zero in the loop
				m_currentOrder = m_jumpToOrder;
				m_jumpToOrder = -1;
			} else if (m_patternBreakToRow>=0){
				m_currentRow = m_patternBreakToRow-1; // subtract 1, because it will be incremented in the loop
				m_currentOrder++;
				if (m_currentOrder>m_lastOrder) m_currentOrder = m_firstOrder;
				m_patternBreakToRow = -1;
			}
			// process pattern loop if necessary
			if (m_patternLoopTo>=0){
				m_currentRow = m_patternLoopTo-1; // subtract 1, because it will be incremented below
				m_patternLoopTo = -1;
			}

			m_currentTick = 0;
			m_currentRow++;
			if (m_currentRow>=64){
				m_currentOrder++;
				if (m_currentOrder>m_lastOrder){
					m_currentOrder = m_firstOrder;
					if (!m_loop) {
						for (int ch=0; ch<m_numChannels; ch++) m_mixer->SetVoiceStatus(ch, false);
						m_reachedEnd = true;
						Stop();
						return;
					}
				}
				m_currentRow = 0;
				m_patternLoopRow = 0;
				m_patternLoopCount = 0;
			}
			DecodeRow(); // Prepare the note's information
			ProcessRow();
			// Process effects specific to the the first tick
			ProcessFirstTickEffects();

		} else {
			m_patternDelay--;
		}
	}

	UpdateEffects();

	UpdateMixer();

}



/*
int MODModule::GetFirstMixerChannel(void){
	return m_firstMixerChannel;
}
*/

int MODModule::GetChannels(void){
	return m_numChannels;
}

int MODModule::GetPosition(void){
	return m_currentOrder;
}

int MODModule::GetOrders(void){
	return m_numOrders;
}

int MODModule::GetPattern(void){
	return m_orders[m_currentOrder];
}

int MODModule::GetPatternRows(void){
	return 64;
}

int MODModule::GetRow(void) {
	return m_currentRow;
}

int MODModule::GetSpeed(void) {
	return m_speed;
}

} // namespace cz::microaudio


#endif // CZMICROAUDIO_MOD_ENABLED
