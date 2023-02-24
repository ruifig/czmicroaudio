//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czStaticSound.h"
#include "czStandardC.h"


namespace cz
{
namespace microaudio
{

// Constructor to czStaticSound objects.
/*
It simply initializes some private variables
*/
StaticSound::StaticSound(::cz::Core *parentObject) : ::cz::Object(parentObject)
{	
	PROFILE();

	m_format=0;
	m_frequency=22050;
	m_vol = AUDIO_VOL_MAX/2;
	m_pan = AUDIO_PAN_MIDDLE;
	m_loopStart=0;
	m_loopLength=0;
	m_length=0;
	m_framesize=0;
	m_frames=0;
	m_startptr=NULL;
	m_allocatedSize=0;
	m_dptr=NULL;
}

// Destructor of czStaticSound objects
/*
*/
StaticSound::~StaticSound()
{
	PROFILE();

	if(m_startptr!=NULL) CZFREE(m_startptr);
}

/* Calculates the samplesize */	
int StaticSound::CalcFrameSize(int format)
{
	//PROFILE();

	int framesize=0;
	if (format&SOUND_ADPCM){
		framesize = 4;
	} else {
		framesize = (format&SOUND_16BITS) ? 16 : 8;
	}
	if(format&SOUND_STEREO) framesize*=2;
	return framesize;
}

int StaticSound::SetDefaults(int freq, int vol, int pan)
{
	if (freq!=-1){
		m_frequency = freq;
	}
	if (vol!=-1){
		if (!INRANGE(vol, 0, AUDIO_VOL_MAX)) CZERROR(ERR_INVPAR);
		m_vol = (uint8_t)vol;
	}
	if (pan!=-1){
		if (!INRANGE(pan, AUDIO_PAN_LEFT, AUDIO_PAN_RIGHT)) CZERROR(ERR_INVPAR);
		m_pan = (uint8_t)pan;
	}
	return ERR_OK;
}

void StaticSound::GetLoopMode(int *loopMode, int *loopStart, int *loopLength)
{
	*loopMode = m_format&(SOUND_LOOP_OFF|SOUND_LOOP_NORMAL|SOUND_LOOP_BIDI);
	*loopStart = m_loopStart;
	*loopLength = m_loopLength;
}

int StaticSound::Set(int form, int frames)
{
	PROFILE();

	int checkBits=form&(SOUND_8BITS|SOUND_16BITS);
	int checkChannels=form&(SOUND_MONO|SOUND_STEREO);
	int checkSign=form&(SOUND_SIGNED|SOUND_UNSIGNED);
	int checkLoop=form&(SOUND_LOOP_OFF|SOUND_LOOP_NORMAL|SOUND_LOOP_BIDI);
	if (checkLoop==0) {
		checkLoop = SOUND_LOOP_OFF; // Assume loop off if none
		form |= SOUND_LOOP_OFF;
	}

	// Check for ambiguities in format.
	if (  ((form&SOUND_ADPCM)&&(!(form&SOUND_16BITS))) ||
		((checkBits!=SOUND_8BITS)&&(checkBits!=SOUND_16BITS)) ||
		((checkChannels!=SOUND_MONO)&&(checkChannels!=SOUND_STEREO)) ||
		((checkSign!=SOUND_SIGNED)&&(checkSign!=SOUND_UNSIGNED)) ||
		((checkLoop!=SOUND_LOOP_OFF)&&(checkLoop!=SOUND_LOOP_NORMAL)&&(checkLoop!=SOUND_LOOP_BIDI))
		){
		CZERROR(ERR_INVPAR);	
	}

	m_format=form;

	m_framesize = CalcFrameSize(form);
	m_frames = frames;
	m_length = (m_framesize*m_frames)/8;
	// calculate the length in ms
	/*
	{
		int tmp = m_frames * 1000;
		m_lengthms = tmp / m_frequency;
		if ((m_frames % m_frequency)!=0) m_lengthms++;    
	}
	*/
	//	repeatpos=-1;

	// Free the memory, if calling this function more than once
	if (m_startptr != NULL){
		CZFREE(m_startptr);
	}

	int safetyAreaBytes = (SOUND_SAFETYAREA*m_framesize)/8;
	m_allocatedSize = sizeof(uint8_t) * (m_length+(safetyAreaBytes*2)); 
	m_startptr = (uint8_t*) CZALLOC(m_allocatedSize);
	if (m_startptr==NULL){
		CZERROR(ERR_NOMEM);
	}	
	m_dptr = (uint8_t*)m_startptr + safetyAreaBytes;
	SetToSilence();
//	m_format |= CZSND_OK;
	m_loopStart = 0;
	m_loopLength = m_frames;

	return ERR_OK;
}


void StaticSound::SmoothLoop(void)
{
	int i;

	if ((m_frames==0)||(IsADPCM())) return;

	int framesToCopy=MIN(SOUND_SAFETYAREA, m_frames);
	int frameSizeBytes = m_framesize/8;

	// Expand the start
	uint8_t *dest  = m_dptr - 1*frameSizeBytes;
	uint8_t *src   = m_dptr + 1*frameSizeBytes;
	for (i=0; i<framesToCopy;i++){
		memcpy(dest, src, frameSizeBytes);
		dest -= frameSizeBytes;
		src += frameSizeBytes;
	}


	// Expand the end
	if (m_format&SOUND_LOOP_OFF){
		dest = m_dptr + m_length;
		src  = m_dptr + m_length - 1*frameSizeBytes;
	} else if (m_format&SOUND_LOOP_NORMAL){
		dest = m_dptr + (m_loopStart+m_loopLength)*frameSizeBytes;
		src  = m_dptr + m_loopStart*frameSizeBytes;
	} else if (m_format&SOUND_LOOP_BIDI){
		dest = m_dptr + (m_loopStart+m_loopLength)*frameSizeBytes;
		src  = m_dptr + (m_loopStart+m_loopLength-1)*frameSizeBytes;
	}

	for (i=0; i<framesToCopy;i++){
		memcpy(dest,src,frameSizeBytes);
		dest += frameSizeBytes;
		if (m_format&SOUND_LOOP_NORMAL){
			src += frameSizeBytes;
		} else if (m_format&SOUND_LOOP_BIDI){
			src -= frameSizeBytes;
		}
	}


	/*
	czDiskFile out;
	out.Open("sample.raw",0,CZFILE_WRITE);
	out.WriteData(m_startptr, m_length+(SAFETYAREA*2)*frameSizeBytes);
	out.Close();
	*/


}

int StaticSound::SetLoopMode(int loopMode, int loopStart, int loopLength)
{	
	if (loopStart+loopLength > m_frames) CZERROR(ERR_INVPAR);

	// Turn off all the bits for loops
	m_format &= ~(SOUND_LOOP_OFF|SOUND_LOOP_NORMAL|SOUND_LOOP_BIDI);
	// Turn on the bits for this loop mode
	m_format |= loopMode;
	m_loopStart = loopStart;
	m_loopLength = loopLength;
	return ERR_OK;
}

int StaticSound::ChangeSign(void)
{
	PROFILE();

	if(m_frames==0) CZERROR(ERR_CANTRUN);
	if(IsSigned()){
		m_format&=~SOUND_SIGNED;
	} else {
		m_format|=SOUND_SIGNED;
	}
	int len = m_frames;

	if(IsStereo()) len*=2;		
	if(Is16Bits()){
		short *p=(short *)m_dptr;
		while(len--){
			*p-=32768;
			p++;
		}
	} else {
		char *p=(char *)m_dptr;
		while(len--){
			*p-=128;
			p++;
		}
	}

	return ERR_OK;				
}		

void StaticSound::SetToSilence()
{
	PROFILE();
	if (m_frames==0)
		return;
	memset(m_startptr, 0, m_allocatedSize);
}

} // namespace microaudio
} // namespace cz
