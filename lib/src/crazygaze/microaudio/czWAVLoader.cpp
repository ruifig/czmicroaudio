//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//


#include "czWAVLoader.h"
#include "czStandardC.h"
#include "czMixer.h"

namespace cz
{
namespace microaudio
{



struct WAVFormatChunk
{
	//char id[4]; // "fmt " - This is already checked in CheckFormat
	uint32_t dataSize;
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t bitsPerSample;
};

struct WAVFactChunk
{
	char id[4]; // "fact"
	int32_t dataSize;
	int32_t numsamples;
};

struct WAVDataChunk
{
	char id[4]; // "data"
	uint32_t dataSize;
};

#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_MS_ADPCM 2
#define WAVE_FORMAT_IMA_ADPCM 17


//
//
// Available Decoders for WAV files
//
//
class WAVDecoder : public ::cz::Object
{
public:
	WAVDecoder(Core *core) : Object(core), m_block(NULL), m_file(NULL), m_fmt(NULL)
	{
		m_numSamplesPerBlock = 0;
	}
	virtual ~WAVDecoder()
	{
		if (m_block)
			CZFREE(m_block);
	}
	void SetParameters(::cz::io::File *file, WAVFormatChunk *fmt)
	{
		m_file = file;
		m_fmt = fmt;
	}
	virtual int ReadExtraFormatBytes(void)
	{
		return ERR_OK;
	}
	// - Decodes a block, using the "Out" method to output samples.
	// - framesToDo - Total number of frames left to decode for the all sound
	virtual void DecodeBlock(const uint8_t *src, int frames)
	{
		CZASSERT(0);
	}

	virtual int DecodeDataChunk(StaticSound *snd, int dataSize)
	{
		// This code can only load mono sounds
		CZASSERT(m_fmt->nChannels==1);
		CZASSERT(m_block==NULL);
		CZASSERT(m_numSamplesPerBlock!=0); // ReadExtraFormatBytes must be called first
		CZASSERT(m_fmt->nBlockAlign>0);

		m_block = (uint8_t*) CZALLOC(m_fmt->nBlockAlign);
		if (m_block==NULL)
			CZERROR(ERR_NOMEM);

		// Initialize m_soundDst, so the Out method works
		m_soundDst = snd->GetPtr();
		m_framesDecoded = 0;

		int numBlocks = dataSize / m_fmt->nBlockAlign;

		while(numBlocks-- && m_framesDecoded<snd->GetNumFrames())
		{
			int err = m_file->ReadData(m_block, m_fmt->nBlockAlign);
			if (err!=ERR_OK)
				CZERROR(err);
			DecodeBlock(m_block, MIN(snd->GetNumFrames()-m_framesDecoded, m_numSamplesPerBlock));
		}

		CZASSERT(m_framesDecoded<=snd->GetNumFrames());
		return ERR_OK;
	}

protected:
	uint8_t *m_block; // Block used to hold temporary data while decoding
	::cz::io::File *m_file;
	WAVFormatChunk *m_fmt;

	void Out(int16_t sample)
	{
		int16_t* ptr = (int16_t*)m_soundDst;
		*ptr++ = sample;
		m_soundDst = ptr;
		m_framesDecoded++;
	}

protected:
	int m_numSamplesPerBlock;

private:
	void *m_soundDst;
	int m_framesDecoded;
};


class PCMDecoder : public WAVDecoder
{
public:
	PCMDecoder(::cz::Core *core) : WAVDecoder(core)
	{
	}
	
	virtual ~PCMDecoder()
	{
	}

	virtual int DecodeDataChunk(StaticSound *snd, int dataSize)
	{
		// This code can only load mono sounds
		CZASSERT(m_fmt->nChannels==1 || m_fmt->nChannels==2) ;

		int err = m_file->ReadData(snd->GetPtr(), dataSize);
		if (err!=ERR_OK)
			CZERROR(err);

		return ERR_OK;
	}
};

class IMAADPCMDecoder : public WAVDecoder
{
public:
	IMAADPCMDecoder(::cz::Core *core) : WAVDecoder(core)
	{
	}
	virtual ~IMAADPCMDecoder()
	{
	}

	virtual int ReadExtraFormatBytes()
	{
		m_numSamplesPerBlock = m_file->ReadUnsigned16();
		return ERR_OK;

	}


	// NOTE : The format for the block header is
	//
	//	int(16)		predictor
	//	byte		initial index
	//	byte		unknown, usually 0 and probablu reserved
	//  .. repeat the same as above if stereo
	//
	//
	/* Algorithm as described in http://multimedia.cx/simpleaudio.html#tth_sEc4.2.2 and http://multimedia.cx/simpleaudio.html#tth_sEc4.2.4
	#  predictor = ((sample1 * coeff1) + (sample2 * coeff2)) / 256
	# predictor += (signed)nibble * idelta (note that nibble is 2's complement)
	# clamp predictor within signed 16-bit range
	# PCM sample = predictor send PCM sample to the output
	# shuffle samples: sample 2 = sample 1, sample 1 = calculated PCM sample
	# compute next adaptive scale factor: idelta = (AdaptationTable[nibble] * idelta) / 256
	# clamp idelta to lower bound of 16
	*/
	virtual void DecodeBlock(const uint8_t *src, int frames)
	{
		CZASSERT(frames>0);

		int32_t predictor=0; // -32768.. 32767
		int32_t stepIndex; // 0..88
		int32_t step; //
	
		predictor = *((int16_t*)(&src[0]));
		stepIndex = src[2];
		if (src[3]!=0)
		{
			// NOTE : src[3] is unused, and should always be 0
			CZASSERT(0);
		}
		src += 4;

		CZASSERT(stepIndex<=88);
		step = m_stepTable[stepIndex];

		Out((int16_t)predictor);
		frames--;
		if (frames==0)
			return;

		bool nibbleSwitch=true;
		while(frames--)
		{
			uint8_t nibble;
			// Get next nibble
			if (nibbleSwitch)
			{
				nibble = (*src) & 0x0f;
			}
			else
			{
				nibble = (*src)>>4;
				src++;
			}
			nibbleSwitch = !nibbleSwitch;

			stepIndex += m_indexTable[nibble];
			if (stepIndex<0)
				stepIndex=0;
			else if (stepIndex>88)
				stepIndex = 88;

			//...
			uint8_t sign = nibble & 8;
			uint8_t delta = nibble & 7;
			int32_t diff = step >> 3;
			if (delta & 4) diff += step;
			if (delta & 2) diff += step >> 1;
			if (delta & 1) diff += step >> 2;
			if (sign)
				predictor -= diff;
			else
				predictor += diff;

			if (predictor>32767)
				predictor = 32767;
			else if (predictor < -32878)
				predictor = -32768;

			step = m_stepTable[stepIndex];

			Out((int16_t)predictor);
		}
	
	}

private:
	static const int8_t m_indexTable[];
	static const int16_t m_stepTable[];
};

/* Intel ADPCM step variation table */
const int8_t IMAADPCMDecoder::m_indexTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

const int16_t IMAADPCMDecoder::m_stepTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


class MSADPCMDecoder : public WAVDecoder
{
public:
	MSADPCMDecoder(Core *core)  : WAVDecoder(core)
	{
		m_numCoefs = 0;
		m_coefs = NULL;
	}

	virtual ~MSADPCMDecoder()
	{
		if (m_coefs)
			CZFREE(m_coefs);
	}

	int ReadExtraFormatBytes()
	{
		int err;
		m_numSamplesPerBlock = m_file->ReadUnsigned16();
		m_numCoefs = m_file->ReadUnsigned16();
		CZASSERT(m_numCoefs>0);
		const int coefsSizeBytes = sizeof(COEFS)*m_numCoefs;
		m_coefs = (COEFS*) CZALLOC(coefsSizeBytes);
		if (m_coefs==NULL)
			CZERROR(ERR_NOMEM);
		if ((err = m_file->ReadData(m_coefs, coefsSizeBytes))!=ERR_OK)
			CZERROR(err);

		return ERR_OK;
	}


	// NOTE : The format for the block header is
	//
	//	BYTE			bPredictor[nChannels];
	//	int(16 bits)	iDelta[nChannels];
	//	int(16 bits)	iSamp1[nChannels];
	//	int(16 bits)	iSamp2[nChannels];
	//
	/* Algorithm as described in http://multimedia.cx/simpleaudio.html#tth_sEc4.3
	#  predictor = ((sample1 * coeff1) + (sample2 * coeff2)) / 256
	# predictor += (signed)nibble * idelta (note that nibble is 2's complement)
	# clamp predictor within signed 16-bit range
	# PCM sample = predictor send PCM sample to the output
	# shuffle samples: sample 2 = sample 1, sample 1 = calculated PCM sample
	# compute next adaptive scale factor: idelta = (AdaptationTable[nibble] * idelta) / 256
	# clamp idelta to lower bound of 16
	*/
	virtual void DecodeBlock(const uint8_t *src, int frames)
	{
		CZASSERT(frames!=0);

		// Read the block header
		const uint8_t blockPredictor = src[0];

		// Now we need to read 3 16-bits values, but they are not aligned (index 1,3,5), which crashes on ARM processors,
		// so we need to use a mem copy instead of just assigning it
		int16_t temp_s16;
		// same as:  int32_t delta = *((int16_t*)(&src[1]))
		memcpy(&temp_s16, &src[1], 2);
		int32_t delta = temp_s16; 
		// same as:  int16_t sample1 = *((int16_t*)(&src[3]));
		memcpy(&temp_s16, &src[3], 2);
		int16_t sample1 = temp_s16; 
		// same as:  int16_t sample2 = *((int16_t*)(&src[5]));
		memcpy(&temp_s16, &src[5], 2);
		int16_t sample2 = temp_s16; 

		src += 7;
		CZASSERT(blockPredictor<m_numCoefs);
		int16_t coef1 = m_coefs[blockPredictor].coef1;
		int16_t coef2 = m_coefs[blockPredictor].coef2;

		//Output the first 2 samples we got from the header itself
		Out(sample2);
		frames--;
		if (frames==0)
			return;
		Out(sample1);
		frames--;

		bool nibbleSwitch=true;
		while(frames--)
		{
			uint8_t nibble;
			// Get next nibble
			if (nibbleSwitch)
			{
				nibble = (*src)>>4;
			}
			else
			{
				nibble = (*src) & 0x0f;
				src++;
			}
			nibbleSwitch = !nibbleSwitch;

			// Convert the nibble value to -8..7
			int8_t signedNibble = (nibble & 8) ? nibble-0x10 : nibble;

			int predictor = ((int)sample1*coef1 + (int)sample2*coef2) / 256;
			predictor += signedNibble * delta;
			predictor = CLAMP(predictor,-32768, 32767);
			Out((int16_t)predictor);
			sample2 = sample1;
			sample1 = (int16_t)predictor;
			delta = (adaptationTable[nibble]*delta)/256;
			if (delta<16)
				delta = 16;	
		}

	}


private:
	static const int32_t adaptationTable[];

	struct COEFS
	{
		int16_t coef1; // 8.8 fixed point
		int16_t coef2; // 8.8 fixed point
	};

	uint16_t m_numCoefs;
	COEFS *m_coefs;

};

const int32_t MSADPCMDecoder::adaptationTable[16] =
{
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};




WAVLoader::WAVLoader(Core *core) : m_core(core)
{
	m_tmpSnd = NULL;
	m_decoder = NULL;
}

WAVLoader::~WAVLoader()
{
	CleanupMemory();
}

void WAVLoader::CleanupMemory()
{
	if (m_tmpSnd)
	{
		CZDELETE(m_tmpSnd);
		m_tmpSnd=NULL;
	}
	if (m_decoder)
	{
		CZDELETE(m_decoder);
		m_decoder = NULL;
	}
}


int WAVLoader::Load(::cz::io::File *in, StaticSound **basesnd)
{
	WAVFormatChunk fmt;
	WAVFactChunk fact;
	WAVDataChunk data;
	int err;

	CleanupMemory();

	if (!CheckFormat(in))
	{
		CZLOG(LOG_ERROR,"Not a WAVE file.\n");
		CZERROR(ERR_WRONGFORMAT);
	}

	// Skip the the amount of bytes we already checked at CheckFormat
	in->Seek(4*4, ::cz::io::FILE_SEEK_START);

	// Read Format Chunk
	memset(&fmt, 0, sizeof(fmt));
	fmt.dataSize = in->ReadUnsigned32();
	fmt.wFormatTag = in->ReadUnsigned16();
	fmt.nChannels = in->ReadUnsigned16();
	fmt.nSamplesPerSec = in->ReadUnsigned32();
	fmt.nAvgBytesPerSec = in->ReadUnsigned32();
	fmt.nBlockAlign = in->ReadUnsigned16();
	fmt.bitsPerSample = in->ReadUnsigned16();

	// Right now, I'm only supporting mono sounds
	if (!(fmt.nChannels==1 || fmt.nChannels==2))
	{
		CZLOG(LOG_ERROR, "Only mono or stereo WAVE files are supported.\n");
		CZERROR(ERR_WRONGFORMAT);
	}

	if (fmt.wFormatTag==WAVE_FORMAT_PCM)
		m_decoder = CZNEW(PCMDecoder)(m_core);
	else if (fmt.wFormatTag==WAVE_FORMAT_MS_ADPCM)
		m_decoder = CZNEW(MSADPCMDecoder)(m_core);
	else if (fmt.wFormatTag==WAVE_FORMAT_IMA_ADPCM)
		m_decoder = CZNEW(IMAADPCMDecoder)(m_core);
	else 
	{
		CZLOG(LOG_ERROR, "Unsupported WAVE encoding.\n");
		CZERROR(ERR_WRONGFORMAT);
	}

	if (m_decoder==NULL)
		CZERROR(ERR_NOMEM);

	m_decoder->SetParameters(in, &fmt);

	if (fmt.wFormatTag!=WAVE_FORMAT_PCM)
	{
		uint16_t extraFormatBytes = in->ReadUnsigned16();
	}

	m_decoder->ReadExtraFormatBytes(); // The format of the extra bytes is format dependent, so call the appropriate virtual function


	// Read Fact chunk if it's not a uncompressed PCM.
	if (fmt.wFormatTag!=WAVE_FORMAT_PCM)
	{
		memset(&fact,0,sizeof(fact));
		in->ReadData(fact.id, 4);
		fact.dataSize = in->ReadSigned32();
		fact.numsamples = in->ReadSigned32();
		if (memcmp(fact.id,"fact",4)!=0)
		{
			CZLOG(LOG_ERROR, "Failed to find expected 'Fact Chunk' in WAVE file.\n");
			CZERROR(ERR_WRONGFORMAT);
		}
	}

	// Read the Data Chunk
	memset(&data, 0, sizeof(data));
	in->ReadData(data.id,4);
	data.dataSize = in->ReadUnsigned32();
	CZASSERT((data.dataSize % fmt.nBlockAlign) == 0);
	if (memcmp(data.id, "data",4)!=0)
	{
		CZLOG(LOG_ERROR, "Failed to find expected 'Data Chunk' in WAVE file.\n");
		CZERROR(ERR_WRONGFORMAT);
	}


	// Calculate the sound size in frames
	const int numFrames = (fmt.wFormatTag==WAVE_FORMAT_PCM) ? (data.dataSize/fmt.nChannels) / (fmt.bitsPerSample/8) : fact.numsamples;

	if ( numFrames > Mixer::AUDIO_MIXER_MAXSAMPLE_SIZE)
	{
		CZLOG(LOG_ERROR, "WAV sound data file exceeds supported sound size\n");
		CZERROR(ERR_NOTAVAILABLE);
	}

	// Create the sound object
	{
		int format=0;
		format|=(fmt.bitsPerSample==16 || fmt.bitsPerSample==4) ? (SOUND_16BITS|SOUND_SIGNED) : (SOUND_8BITS|SOUND_UNSIGNED);
		format|=(fmt.nChannels==2) ? SOUND_STEREO: SOUND_MONO;

		m_tmpSnd = CZNEW(StaticSound)(m_core);
		if (m_tmpSnd==NULL)
			CZERROR(ERR_NOMEM);
		err = m_tmpSnd->Set(format, numFrames);

		if (err!=ERR_OK)
		{
			CZDELETE(m_tmpSnd);
			m_tmpSnd=0;
			CZERROR(err);
		}

	}

	m_decoder->DecodeDataChunk(m_tmpSnd, data.dataSize);
	
	// Set the defaults we can get from the file itself
	m_tmpSnd->SetDefaults((int)fmt.nSamplesPerSec, AUDIO_VOL_MAX, AUDIO_PAN_MIDDLE);

	// We can only use signed formats
	if (!m_tmpSnd->IsSigned())
		m_tmpSnd->ChangeSign();

	*basesnd = m_tmpSnd;
	m_tmpSnd = NULL; // set to NULL so it won't be deleted in CleanUpMemory

	return ERR_OK;
}

bool WAVLoader::CheckFormat(::cz::io::File *in)
{
	// Variable to hold some of the first bytes in the file, enough to quickly test if it's a WAV file
	// 4 bytes - ChunkID ("RIFF")
	// 4 bytes - Chunk Data Size
	// 4 bytes - RIFF Type ID ("WAVE")
	// 4 bytes - ChunkID ("fmt ")
	const int dsize = 4+4+4+4;
	uint8_t header[dsize];

	in->Seek(0, ::cz::io::FILE_SEEK_START);

	memset(header, 0, sizeof(header));

	if (in->ReadData(header, dsize)!=ERR_OK)
		return false;

	if (memcmp(header, "RIFF", 4)==0 && memcmp(&header[8], "WAVEfmt ",8)==0)
		return true;
	else
		return false;
}


} // namespace microaudio
} // namespace cz
