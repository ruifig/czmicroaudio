//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czMixer.h"
#include "czStaticSound.h"
#include "czStream.h"
#include "czStandardC.h"

//#define PROFILE() PROFILE_OFF


/*
bool first=true;
czMophunFile raw;
*/

namespace cz
{
	
namespace audio
{


inline int MakeFixed(uint32_t dividend,uint16_t divisor);
extern "C" MXFUNC Mix8Mono_Normal;
extern "C" MXFUNC Mix8Stereo_Normal;
extern "C" MXFUNC Mix16Mono_Normal;
extern "C" MXFUNC Mix16Stereo_Normal;

//-----------------------------------------------------------------
////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------
//		MIXING FUNCTIONS
//
//
// FINAL OUTPUT TO 8 BITS
void Mixer::MakeOutput(uint8_t *dest,uint32_t todo)
{	
	PROFILE();

	int32_t *src=m_mixblock;
	int32_t c;
	while(todo--){
		c=*src>>(VOLFRACBITS+8);
		if(c>127)
			*dest=255;
		else if(c<-128)
			*dest=0;
		else
			*dest=uint8_t(c+128);
		dest++;
		src++;
	}
}

//
// FINAL OUTPUT TO 16 BITS
void Mixer::MakeOutput(uint16_t *dest,uint32_t todo)
{
	PROFILE();

	int32_t *src=m_mixblock;
	int32_t s;
	while(todo--){
		s=*src>>VOLFRACBITS;
		if(s>32767)
			*dest=32767;
		else if(s<-32768)
			*dest=-32768;
		else
			*dest=uint16_t(s);
		src++;
		dest++;
	}
}


//
//	Converts the fraction 'dividend/divisor' into a fixed point longword.
//
int MakeFixed(uint32_t dividend,uint16_t divisor);
inline int MakeFixed(uint32_t dividend,uint16_t divisor)
{
	//PROFILE();
					
	uint32_t whole,part;

	whole=dividend/divisor;
	part=((dividend%divisor)<<Mixer::FREQFRACBITS)/divisor;
	//CZLOG("WHOLE %u PART %u = %d\n", whole, part, uint32_t((whole<<FREQFRACBITS)|part));

	uint32_t res = (whole<<Mixer::FREQFRACBITS)|part;

	res -= res>>31;

	return res;
}

int MakeSignedFixed(int32_t dividend, int16_t divisor)
{
	int32_t whole,part, res;

	if ((dividend<0 && divisor>0) || (dividend>0 && divisor<0) )
	{
		whole=(-dividend)/divisor;
		part=(((-dividend)%divisor)<<Mixer::FREQFRACBITS)/divisor;
		res = (whole<<Mixer::FREQFRACBITS)|part;
		res = -res;
	}
	else
	{
		whole=dividend/divisor;
		part=((dividend%divisor)<<Mixer::FREQFRACBITS)/divisor;
		res = (whole<<Mixer::FREQFRACBITS)|part;
	}

	return res;
}


#ifndef ASM_MIXER

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//*-*-*-*-*-*-*-*-*-*-*-NON INTERPOLATED MIXING *-*-*-*-*-*-*-*-*-*-*-
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

uint32_t Mix8_Mono_To_Mono_Normal(
		int32_t *dest,
		void *src,
		int32_t pos,
		int32_t inc,
		VOLUME_STATE *volstate,
		int32_t portion)
{
	//PROFILE();
            
	int remain = portion & 0x07;
	int runs = portion >> 3;

	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;

//    int16_t tmpvol = lvol >> 6;
   
    while(runs--){
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;		
	}


	while(remain--){
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
	}

	return pos;

}

//
//  8 BITS MONO TO STEREO
//
uint32_t Mix8_Mono_To_Stereo_Normal(
		int32_t *dest,
		void *src,
		int32_t pos,
		int32_t inc,
		VOLUME_STATE *volstate,
		int32_t portion)
{

	//PROFILE();

//    int16_t tmpvoll = lvol >> 6; 
//    int16_t tmpvolr = rvol >> 6; 
    

	int remain = portion & 0x03;
	int runs = portion >> 2;
	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
	int32_t rvol = (volstate->currRvol>>Mixer::FREQFRACBITS)*256;
	
	while(runs--){
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		*dest += rvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;		
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		*dest += rvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;		
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		*dest += rvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;		
		pos+=inc;
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		*dest += rvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;		
		pos+=inc;		
	}
	
	while (remain--){
		*dest += lvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		*dest += rvol * ((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;		
		pos+=inc;		
	}
	
	return pos;
}

//  8 BITS STEREO TO MONO
//
// TODO : This was hacked to implement ogg vorbis streaming, since I didn't had support for stere sounds
uint32_t Mix8_Stereo_To_Mono_Normal(
								   int32_t *dest,
								   void *src,
								   int32_t pos,
								   int32_t inc,
								   VOLUME_STATE *volstate,
								   int32_t portion)
{

	//PROFILE();

	//    int32_t tmpvoll = lvol >> 6; 
	//    int32_t tmpvolr = rvol >> 6; 

	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;

	int32_t l,r;
	while(portion--){
		l = (int32_t)((int8_t *)src)[(pos>>Mixer::FREQFRACBITS)<<1];
		r = (int32_t)((int8_t *)src)[((pos>>Mixer::FREQFRACBITS)<<1) +1];
		*dest+= lvol * ((l+r)>>1); // Addleft and right, and divide by 2
		dest++;
		pos+=inc;
	}
	return(pos);
}

//
//  8 BITS STEREO TO STEREO
//
// TODO : This was hacked to implement ogg vorbis streaming, since I didn't had support for stereo sounds
uint32_t Mix8_Stereo_To_Stereo_Normal(
									  int32_t *dest,
									  void *src,
									  int32_t pos,
									  int32_t inc,
									  VOLUME_STATE *volstate,
									  int32_t portion)
{

	//PROFILE();

	//    int32_t tmpvoll = lvol >> 6; 
	//    int32_t tmpvolr = rvol >> 6; 

	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
	int32_t rvol = (volstate->currRvol>>Mixer::FREQFRACBITS)*256;

	while(portion--){
		*dest+= lvol * (int32_t)((int8_t *)src)[(pos>>Mixer::FREQFRACBITS)<<1];
		dest++;
		*dest+= rvol * (int32_t)((int8_t *)src)[((pos>>Mixer::FREQFRACBITS)<<1) +1];
		dest++;
		pos+=inc;
	}
	return(pos);
}



//
//  16 BITS MONO TO MONO
//
uint32_t Mix16_Mono_To_Mono_Normal(
		int32_t *dest,
		void *src,
		int32_t pos,
		int32_t inc,
		VOLUME_STATE *volstate,
		int32_t portion)
{

	//PROFILE();

//    int tmpvol = lvol >> 6;
	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;

	while(portion--){
		*dest+= lvol * (int32_t)((short *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		}
	return(pos);
}
//
//  16 BITS MONO TO STEREO
//
uint32_t Mix16_Mono_To_Stereo_Normal(
		int32_t *dest,
		void *src,
		int32_t pos,
		int32_t inc,
		VOLUME_STATE *volstate,
		int32_t portion)
{

	//PROFILE();

//    int32_t tmpvoll = lvol >> 6; 
//    int32_t tmpvolr = rvol >> 6; 

	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
	int32_t rvol = volstate->currRvol>>Mixer::FREQFRACBITS;

	while(portion--){
		*dest+= lvol * (int32_t)((short *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		*dest+= rvol * (int32_t)((short *)src)[pos>>Mixer::FREQFRACBITS];
		dest++;
		pos+=inc;
		}
	return(pos);
}

//  16 BITS STEREO TO MONO
//
// TODO : This was hacked to implement ogg vorbis streaming, since I didn't had support for stere sounds
uint32_t Mix16_Stereo_To_Mono_Normal(
									  int32_t *dest,
									  void *src,
									  int32_t pos,
									  int32_t inc,
									  VOLUME_STATE *volstate,
									  int32_t portion)
{

	//PROFILE();

	//    int32_t tmpvoll = lvol >> 6; 
	//    int32_t tmpvolr = rvol >> 6; 

	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
	int32_t l,r;
	while(portion--){
		l = (int32_t)((short *)src)[(pos>>Mixer::FREQFRACBITS)<<1];
		r = (int32_t)((short *)src)[((pos>>Mixer::FREQFRACBITS)<<1) +1];
		*dest+= lvol * ((l+r)>>1); // Addleft and right, and divide by 2
		dest++;
		pos+=inc;
	}
	return(pos);
}


//
//  16 BITS STEREO TO STEREO
//
// TODO : This was hacked to implement ogg vorbis streaming, since I didn't had support for stere sounds
uint32_t Mix16_Stereo_To_Stereo_Normal(
									  int32_t *dest,
									  void *src,
									  int32_t pos,
									  int32_t inc,
									  VOLUME_STATE *volstate,
									  int32_t portion)
{

	//PROFILE();

	//    int32_t tmpvoll = lvol >> 6; 
	//    int32_t tmpvolr = rvol >> 6; 

	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
	int32_t rvol = volstate->currRvol>>Mixer::FREQFRACBITS;

	while(portion--){
		*dest+= lvol * (int32_t)((short *)src)[(pos>>Mixer::FREQFRACBITS)<<1];
		dest++;
		*dest+= rvol * (int32_t)((short *)src)[((pos>>Mixer::FREQFRACBITS)<<1) +1];
		dest++;
		pos+=inc;
	}
	return(pos);
}

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//*-*-*-*-*-*-*-*-*-*-*-LINEAR INTERPOLATION MIXING *-*-*-*-*-*-*-*-
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

uint32_t Mix8_Mono_To_Mono_Linear(
		int32_t *dest,
		void *src,
		int32_t pos,
		int32_t inc,
		VOLUME_STATE *volstate,
		int32_t portion)
{
	//PROFILE();
	int32_t s,a,b;
	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;

	while(portion--){
		a=((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		b=((int8_t *)src)[1+(pos>>Mixer::FREQFRACBITS)];
		s=a+(((int32_t)(b-a)*(int32_t)(pos&Mixer::FREQFRACMASK))>>Mixer::FREQFRACBITS);
		
		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
		}

		*dest+=s*lvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s*lvol;

	return pos;
}

//
//  8 BITS MONO TO STEREO
//
uint32_t Mix8_Mono_To_Stereo_Linear(
		int32_t *dest,
		void *src,
		int32_t pos,
		int32_t inc,
		VOLUME_STATE *volstate,
		int32_t portion)
{

	int32_t s,a,b;	
	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
	int32_t rvol = (volstate->currRvol>>Mixer::FREQFRACBITS)*256;

	while(portion--){
		a=(int)((int8_t *)src)[pos>>Mixer::FREQFRACBITS];
		b=(int)((int8_t *)src)[1+(pos>>Mixer::FREQFRACBITS)];
		s=a+(((int32_t)(b-a)*(int32_t)(pos&Mixer::FREQFRACMASK))>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			volstate->currRvol += volstate->incRvol;
			lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
			rvol = (volstate->currRvol>>Mixer::FREQFRACBITS)*256;
		}

		*dest+=s*lvol;
		dest++;
		*dest+=s*rvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s*lvol;
	volstate->lastSampleMixedR = s*rvol;

	return pos;
}

//
//  8 BITS STEREO TO Mono
//
uint32_t Mix8_Stereo_To_Mono_Linear(
									int32_t *dest,
									void *src,
									int32_t pos,
									int32_t inc,
									VOLUME_STATE *volstate,
									int32_t portion)
{
	//PROFILE();
	int32_t s,a,b;
	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;

	int8_t *p;
	while(portion--){
		p = &((int8_t *)src)[(pos>>Mixer::FREQFRACBITS)<<1];
		a = ((int32_t)p[0] + (int32_t)p[1]) >> 1; // Mix left and right
		b = ((int32_t)p[2] + (int32_t)p[3]) >> 1; // Mix left and right
		s=a+(((int32_t)(b-a)*(int32_t)(pos&Mixer::FREQFRACMASK))>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
		}

		*dest+=s*lvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s*lvol;

	return pos;
}


//
//  8 BITS STEREO TO STEREO
//
uint32_t Mix8_Stereo_To_Stereo_Linear(
									 int32_t *dest,
									 void *src,
									 int32_t pos,
									 int32_t inc,
									 VOLUME_STATE *volstate,
									 int32_t portion)
{
	//PROFILE();
	int32_t s1,s2,a,b;

	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
	int32_t rvol = (volstate->currRvol>>Mixer::FREQFRACBITS)*256;
	int32_t c;
	int8_t* p;
	while(portion--){
		p = &(((int8_t *)src)[(pos>>Mixer::FREQFRACBITS)<<1]);
		c = (int32_t)(pos&Mixer::FREQFRACMASK);
		a= *p;
		b= *(p+2);
		s1=a+(((b-a)*c)>>Mixer::FREQFRACBITS);
		a= *(p+1);
		b= *(p+3);
		s2=a+(((b-a)*c)>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			volstate->currRvol += volstate->incRvol;
			lvol = (volstate->currLvol>>Mixer::FREQFRACBITS)*256;
			rvol = (volstate->currRvol>>Mixer::FREQFRACBITS)*256;
		}

		*dest+=s1*lvol;
		dest++;
		*dest+=s2*rvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s1*lvol;
	volstate->lastSampleMixedR = s2*rvol;

	return pos;}

//
//  16 BITS MONO TO MONO
//
uint32_t Mix16_Mono_To_Mono_Linear(
								  int32_t *dest,
								  void *src,
								  int32_t pos,
								  int32_t inc,
								  VOLUME_STATE *volstate,
								  int32_t portion)
{

	//PROFILE();
	int32_t s,a,b;

	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;

	while(portion--){
		a=((int16_t *)src)[pos>>Mixer::FREQFRACBITS];
		b=((int16_t *)src)[1+(pos>>Mixer::FREQFRACBITS)];
		s=a+(((int32_t)(b-a)*(int32_t)(pos&Mixer::FREQFRACMASK))>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			volstate->currRvol += volstate->incRvol;
			lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
		}

		*dest+=s*lvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s*lvol;

	return pos;
}
//
//  16 BITS MONO TO STEREO
//
uint32_t Mix16_Mono_To_Stereo_Linear(
									int32_t *dest,
									void *src,
									int32_t pos,
									int32_t inc,
									VOLUME_STATE *volstate,
									int32_t portion)
{

	//PROFILE();
	int32_t s,a,b;

	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
	int32_t rvol = volstate->currRvol>>Mixer::FREQFRACBITS;

	while(portion--){
		a=((int16_t *)src)[pos>>Mixer::FREQFRACBITS];
		b=((int16_t *)src)[1+(pos>>Mixer::FREQFRACBITS)];
		s=a+(((int32_t)(b-a)*(int32_t)(pos&Mixer::FREQFRACMASK))>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			volstate->currRvol += volstate->incRvol;
			lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
			rvol = volstate->currRvol>>Mixer::FREQFRACBITS;
		}

		*dest+=s*lvol;
		dest++;
		*dest+=s*rvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s*lvol;
	volstate->lastSampleMixedR = s*rvol;

	return pos;
}



//
//  16 BITS STEREO TO Mono
//
uint32_t Mix16_Stereo_To_Mono_Linear(
									int32_t *dest,
									void *src,
									int32_t pos,
									int32_t inc,
									VOLUME_STATE *volstate,
									int32_t portion)
{
	//PROFILE();
	int32_t s,a,b;
	int32_t lvol = (volstate->currLvol>>Mixer::FREQFRACBITS);

	int16_t *p;
	while(portion--){
		p = &((int16_t *)src)[(pos>>Mixer::FREQFRACBITS)<<1];
		a = ((int32_t)p[0] + (int32_t)p[1]) >> 1; // Mix left and right
		b = ((int32_t)p[2] + (int32_t)p[3]) >> 1; // Mix left and right
		s=a+(((int32_t)(b-a)*(int32_t)(pos&Mixer::FREQFRACMASK))>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
		}

		*dest+=s*lvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s*lvol;

	return pos;
}



//
//  16 BITS STEREO TO STEREO
//
uint32_t Mix16_Stereo_To_Stereo_Linear(
									  int32_t *dest,
									  void *src,
									  int32_t pos,
									  int32_t inc,
									  VOLUME_STATE *volstate,
									  int32_t portion)
{
	//PROFILE();
	int32_t s1,s2,a,b;

	int32_t lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
	int32_t rvol = volstate->currRvol>>Mixer::FREQFRACBITS;
	int32_t c;
	int16_t* p;
	while(portion--){
		p = &(((int16_t *)src)[(pos>>Mixer::FREQFRACBITS)<<1]);
		c = (int32_t)(pos&Mixer::FREQFRACMASK);
		a= *p;
		b= *(p+2);
		s1=a+(((b-a)*c)>>Mixer::FREQFRACBITS);
		a= *(p+1);
		b= *(p+3);
		s2=a+(((b-a)*c)>>Mixer::FREQFRACBITS);

		if (volstate->volrampcount)
		{
			volstate->volrampcount--;
			volstate->currLvol += volstate->incLvol;
			volstate->currRvol += volstate->incRvol;
			lvol = volstate->currLvol>>Mixer::FREQFRACBITS;
			rvol = volstate->currRvol>>Mixer::FREQFRACBITS;
		}

		*dest+=s1*lvol;
		dest++;
		*dest+=s2*rvol;
		dest++;
		pos+=inc;
	}

	volstate->lastSampleMixedL = s1*lvol;
	volstate->lastSampleMixedR = s2*rvol;

	return pos;
}


#endif		

void Mixer::SetStopRamp(CHANNEL *chptr)
{
	CZASSERT(QualityType>AUDIO_INTERPOLATION_NONE);
	chptr->stopRampState.restart = false;

	if (stopRampingNumFrames && (chptr->volState.lastSampleMixedL!=0 || chptr->volState.lastSampleMixedR))
	{
		chptr->stopRampState.stoprampcount = stopRampingNumFrames;
		chptr->stopRampState.currLval = chptr->volState.lastSampleMixedL;
		chptr->stopRampState.currRval = chptr->volState.lastSampleMixedR;
		chptr->stopRampState.incLval = -chptr->volState.lastSampleMixedL / stopRampingNumFrames;
		chptr->stopRampState.incRval = -chptr->volState.lastSampleMixedR / stopRampingNumFrames;
	}
}


void Mixer::Mix_StopRamp(int32_t *dest, STOPRAMP_STATE *stoprampstate, int32_t portion)
{
	CZASSERT(QualityType>AUDIO_INTERPOLATION_NONE);

	int todo = MIN(portion, stoprampstate->stoprampcount);
	stoprampstate->stoprampcount -= todo;

	if (IS_STEREO)
	{
		while(todo--)
		{
			*dest += stoprampstate->currLval;
			dest++;
			stoprampstate->currLval += stoprampstate->incLval;

			*dest += stoprampstate->currRval;
			dest++;
			stoprampstate->currRval += stoprampstate->incRval;
		}
	}
	else
	{
		while(todo--)
		{
			*dest += stoprampstate->currLval;
			stoprampstate->currLval += stoprampstate->incLval;
		}
	}

}


//-----------------------------------------------------------------
////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------
//                   CONTROLS CHANNEL MIXING
//-----------------------------------------------------------------
void Mixer::MixChannel(CHANNEL *chptr,uint32_t len, int32_t *mixbuff)
{
	
	PROFILE();

	uint32_t lvol,rvol;
	int inc=chptr->increment;
	int incMODULE = inc;

	int todo=len;
	int32_t *mxpos = mixbuff;
	int pos=chptr->pos;
	int end = chptr->end;
	int repeat=chptr->repeat;
	int portion;
	//uint8_t check=0;

	if(chptr->back) inc=-inc;

	// Calculate left and right volume;
	uint32_t cvol=(uint16_t(chptr->vol)*uint16_t(chptr->mastervol));	
	uint32_t pan= chptr->panning;
	if(!IS_STEREO) pan=128;
	if(pan==0){
		lvol=cvol;
		rvol=0;
	} else if(pan==128) {
		//lvol=rvol=cvol/2; // TODO : This is probably wrong. We should not divide, because it will sound too quiet.
		lvol=rvol=cvol;
	} else if(pan==255){
		rvol=cvol;
		lvol=0;
	} else{	
		uint32_t tmp=(uint32_t(cvol)*uint32_t(pan))/256;
		rvol=(uint16_t)tmp;
		lvol=cvol-rvol;	
	}

	//
	// Prepare volume ramping
	//
	if (volumeRampingNumFrames!=0 && QualityType>AUDIO_INTERPOLATION_NONE)
	{
		int32_t lvolFixed = lvol << FREQFRACBITS;
		int32_t rvolFixed = rvol << FREQFRACBITS;
		if (chptr->volState.restart) // restart ramping if necessary
		{
			chptr->volState.restart = false;
			chptr->volState.currLvol = 0;
			chptr->volState.currRvol = 0;
			chptr->volState.destLvol = 0;
			chptr->volState.destRvol = 0;
		}

		if (chptr->volState.destLvol!=lvolFixed || chptr->volState.destRvol!=rvolFixed) // recalculate increment the final volume changed
		{
			chptr->volState.volrampcount = volumeRampingNumFrames;
			chptr->volState.incLvol = MakeSignedFixed( lvol-(chptr->volState.currLvol>>FREQFRACBITS), volumeRampingNumFrames);
			chptr->volState.incRvol = MakeSignedFixed( rvol-(chptr->volState.currRvol>>FREQFRACBITS), volumeRampingNumFrames);
			chptr->volState.destLvol = lvol<<FREQFRACBITS;
			chptr->volState.destRvol = rvol<<FREQFRACBITS;
		}
	}
	else
	{
		chptr->volState.currLvol = lvol<<FREQFRACBITS;
		chptr->volState.currRvol = rvol<<FREQFRACBITS;
	}

	while(todo>0)
	{
		// Calculate stuff if going forward
		if (chptr->back==0){

			// Check if positions needs fixing
			if (pos>=end)
			{
				switch (chptr->lloop){
					case SOUND_LOOP_OFF:
						chptr->on=0;
						chptr->pos=0;
						// Mix the rest with the stop ramp if any
						if (QualityType>AUDIO_INTERPOLATION_NONE)
						{
							SetStopRamp(chptr);
							if (chptr->stopRampState.stoprampcount)
							{
								Mix_StopRamp(mxpos, &chptr->stopRampState, todo);
							}
						}
						return;
					case SOUND_LOOP_NORMAL:
						pos = pos - (end-repeat);
						break;
					case SOUND_LOOP_BIDI:
						// calculate the overrun
						pos -=inc;
						
						// this condition should never happen, but if it does, we can fix it.
						if (pos>=end)
							pos = end-1;

						//pos = end - (pos-end);
						inc=-inc;
						chptr->back=1;
						break;
				}
			}

		}
		else // Calculate necessary stuff if going backwards
		{
			// Check if position needs fixing
			if (pos<repeat)
			{
				pos += incMODULE;
				// This condition should never happen, but if it does, we can fix it
				if (pos<repeat)
					pos = repeat;
				inc = -inc;
				chptr->back=0;
			}

		}

#if CZ_DEBUG
		// By this point, pos must be valid already
		CZASSERT(pos>=0 && pos<end);
#endif


		// Calculate number of samples we can mix before hitting the sample's end
		if (chptr->back==0)
		{
			// Check using inc*(todo-1), because that's the last sample the mixing routines try to use
			// < end, because "end" is already an invalid sample position
			if ((pos+(inc*(todo-1)))<end)
			{
				portion = todo;
			}
			else
			{
				portion = (end-pos)/incMODULE;
				if ((end-pos)%incMODULE)
					portion++;
			}			
		}
		else
		{
			// Check using inc*(todo-1), because that's the last sample the mixing routines try to use
			// also, >=repeat, because =repeat is still a valid sample position
			if (pos+(inc*(todo-1))>=repeat) 
			{
				portion = todo;
			}
			else
			{
				portion = ((pos-repeat) / incMODULE) + 1;
				/*
				if ((pos-repeat) % incMODULE)
					portion++;
				*/
			}
		}



		// Make some debug asserts to make sure everything is ok, like
		// checking positions, portions to mix, etc
#if CZ_DEBUG
		CZASSERT(portion>0);
		if (chptr->back==0)
		{
			CZASSERT(inc>0 && (pos+(inc*(portion-1))<end) );
		}
		else
		{			
			CZASSERT(inc<0 && (pos>=repeat) && (pos+(inc*(portion-1))>=repeat))
		}
#endif

		void *wave=chptr->snd->GetPtr();

		if(portion)
		{
			//pos=chptr->mixfunc(mxpos,wave,pos,inc,lvol,rvol,portion);
			if (chptr->mixingListener)
			{
				bool keepMixing = chptr->mixingListener->ChannelMix(pos>>FREQFRACBITS, (portion*inc)>>FREQFRACBITS);
				if (!keepMixing)
				{
					chptr->on=0;
					break;
				}
			}
			pos = chptr->mixfunc(mxpos, wave, pos, inc, &chptr->volState, portion);
		}

		todo-=portion;
		mxpos+=(portion*samplesize);
	}

	chptr->pos=pos;
}


/*******************************************************************
 *******************************************************************
 ********                       INTERFACE                   ********
 *******************************************************************
 *******************************************************************/


// Destructor for a czMixer object
/*
 Simply releases any allocated resources.
*/
void Mixer::Free(void)
{
	PROFILE();
	
	if(m_mixblock!=NULL){
		CZFREE(m_mixblock);
		m_mixblock=NULL;
	}
	if(channels!=NULL){
		CZFREE(channels);
		channels=NULL;
	}
}


Mixer::Mixer(::cz::Core *parentObject) : ::cz::Object(parentObject)
{
    PROFILE();

	m_mixblock = NULL;
	channels = NULL;
	volumeRampingNumFrames = 0;
	stopRampingNumFrames = 0;
}

Mixer::~Mixer(){
    PROFILE();

	Free();
}


// Initializes a czMixer object.
/*
 Must be called before any other member functions.

\arg \c numberofchannels
Maximum number of channels the mixer should support. There is no need to set 
this too high. e.g: choose the minimum number of channels you will need.
More channels will require more memory and processing power if they go active. 
A reasonable number is 32 channels.

\arg \c mixsize
It sets the size in frames of the internal mixing block.
E.g: A value of 22050 with a frequency of 44100 the longest mix can have 
1/2 seconds of sound.

\arg \c stereo
Interpreted as a boolean value. If TRUE(non-zero) Stereo output is selected. 
If FALSE, output will be mono.
\arg \c is16bits
Interpreted as a bollean value. If TRUE(non-zero), then output will be in 
16 bits. If FALSE, then output is 8 bits.

\arg \c freq
Frequency of the output.
A higher frequency require more proccessing power, because more bytes must 
be processed, but quality is higher also.
A lower frequency requires less processing power, but decreases quality.

\retval JOK
Ok.
\retval JNOMEM
Not enough memory to allocate internal buffers.

\warning
You MUST call this member function before using the object.
All the other member functions assume this as been called, and so no check 
is made.
*/
int Mixer::Init(uint32_t numberofchannels, uint32_t mixsize,
		bool stereo, bool is16bits, uint16_t freq)
{
	PROFILE();
	
	nch=numberofchannels;

	IS_16BITS=(is16bits);
	IS_STEREO=(stereo);
	FREQUENCY=freq;
	samples=mixsize;

	samplesize=1;
	if(IS_STEREO) samplesize*=2;

	if(m_mixblock!=NULL) CZFREE(m_mixblock);
	if(channels!=NULL) CZFREE(channels);

	// allocate mixing buffer only if 8 bits output is used
	m_mixblock = (int32_t*) CZALLOC(sizeof(int32_t)*mixsize*samplesize);
	if (m_mixblock==NULL) CZERROR(ERR_NOMEM);
	int length=sizeof(m_mixblock[0])*mixsize*samplesize;
	memset(m_mixblock,0,length);
	
	channels=(CHANNEL*) CZALLOC(sizeof(CHANNEL)*nch);
	if(channels==NULL) CZERROR(ERR_NOMEM);
	memset(channels,0,sizeof(CHANNEL)*nch);

	SetQuality(AUDIO_INTERPOLATION_NONE);

	return ERR_OK;
}


void Mixer::SetVolumeRamping(int numFrames)
{
	volumeRampingNumFrames = numFrames;
}
void Mixer::SetStopRamping(int numFrames)
{
	stopRampingNumFrames = numFrames;
}

// Change quality mode
/*
 Changes the sound quality level.
 By default, Init() sets the quality level to ::NO_INTERPOLATION.

\arg \c qtype
 See ::CZPLAYER_MIXERQUALITY

\retval JOK
Ok.

\sa Init(), ::CZPLAYER_MIXERQUALITY
*/
int Mixer::SetQuality(AudioInterpolationMode qtype)
{
	PROFILE();
	
	qtype=qtype;
	void *old16Stereo=(void*)Mix16StereoFunc;
	void *old8Stereo=(void*)Mix8StereoFunc;
	void *old16Mono=(void *)Mix16MonoFunc;
	void *old8Mono=(void *)Mix8MonoFunc;

	QualityType=qtype;
	
	switch(qtype){
		case AUDIO_INTERPOLATION_CUBIC:
		case AUDIO_INTERPOLATION_LINEAR:
			if(IS_STEREO){
				Mix16StereoFunc=Mix16_Stereo_To_Stereo_Linear;
				Mix8StereoFunc=Mix8_Stereo_To_Stereo_Linear;
				Mix16MonoFunc=Mix16_Mono_To_Stereo_Linear;
				Mix8MonoFunc=Mix8_Mono_To_Stereo_Linear;
			}
			else{
				Mix16StereoFunc=Mix16_Stereo_To_Mono_Linear;
				Mix8StereoFunc=Mix8_Stereo_To_Mono_Linear;
				Mix16MonoFunc=Mix16_Mono_To_Mono_Linear;
				Mix8MonoFunc=Mix8_Mono_To_Mono_Linear;
			}
			break;
		default:
			QualityType=AUDIO_INTERPOLATION_NONE;
			if(IS_STEREO){
				Mix16StereoFunc=Mix16_Stereo_To_Stereo_Normal;
				Mix8StereoFunc=Mix8_Stereo_To_Stereo_Normal;
				Mix16MonoFunc=Mix16_Mono_To_Stereo_Normal;
				Mix8MonoFunc=Mix8_Mono_To_Stereo_Normal;
			}
			else{
				Mix16StereoFunc=Mix16_Stereo_To_Mono_Normal;
				Mix8StereoFunc=Mix8_Stereo_To_Mono_Normal;
				Mix16MonoFunc=Mix16_Mono_To_Mono_Normal;
				Mix8MonoFunc=Mix8_Mono_To_Mono_Normal;
			}
			break;
	}


	CHANNEL *chptr=&channels[0];
	for(int c=0;c<nch;c++)
	{
		if(chptr->mixfunc==old16Stereo) 
			chptr->mixfunc=Mix16StereoFunc;
		else if (chptr->mixfunc==old8Stereo)
			chptr->mixfunc=Mix8StereoFunc;
		else if (chptr->mixfunc==old16Mono)
			chptr->mixfunc=Mix16MonoFunc;
		else if (chptr->mixfunc==old8Mono)
			chptr->mixfunc=Mix8MonoFunc;
		chptr++;
	}
		
	return ERR_OK;	
}

AudioInterpolationMode Mixer::GetQuality()
{
	return QualityType;
}


// Mix a given number of samples.
/*
 This member function mixs a given number of samples to a specified memory 
location.

\arg \c dest
Pointer to memory allocated by you, to where to write the ouput.
Make sure you know the output format, and you have allocated enough memory to 
store all the samples.

\arg \c len
Number of samples to mix to output (mix).
It must be less or equal the maximum number of samples supported ( see Init() ).

\retval JOK
Ok.
\retval JINVPAR
Invalid \c len (greater than the one passed in Init() )

\sa Init(), SetQualityType()
*/

int Mixer::MixPortion(void *dest, uint32_t len)
{
	
	PROFILE();
	
CHANNEL *chptr=&channels[0];

#if CZ_DEBUG
	if(len>samples) CZERROR(ERR_INVPAR);
#endif

	memset(m_mixblock,0,len*sizeof(m_mixblock[0])*samplesize); // erase only what will be used
	
	for(int c=0;c<nch;c++)
	{
//		if((chptr->repeat<0)||(chptr->pos>=chptr->end))
//		if((chptr->repeat<0)||(chptr->pos>=chptr->end))
//			chptr->on=0;

		//
		// Start a stop ramp if necessary
		//
		if (QualityType>AUDIO_INTERPOLATION_NONE)
		{
			if (chptr->stopRampState.restart)
				SetStopRamp(chptr);
			//
			// Mix in the sample stop ramp if any
			if (chptr->stopRampState.stoprampcount)
			{
				Mix_StopRamp(m_mixblock, &chptr->stopRampState, len);
			}
		}

		if((chptr->on)&&(chptr->snd!=NULL))
			MixChannel(chptr,len, m_mixblock);

		chptr++;
	}

	int todo=len;
	if(IS_STEREO) todo*=2;
	if(IS_16BITS){
		MakeOutput((uint16_t *)dest,todo);
	} else {
		MakeOutput((uint8_t *)dest,todo);
	}
    


	return ERR_OK;		
}



// Set a channel (i.e,voice)
/*
 Use this member function to set all the parameters of a channel.
 The same thing can be done by calling the other simpler member functions, 
like SetSample(), SetFrequency(), etc.

 The channel is set as disabled. You must use SetVoiceStatus() to enable it.

\arg \c ch
 The channel to set the parameters. It ranges from 0...(\c numberofchannels-1), 
where \c numberofchannels is set in Init().

\arg \c sound
 Pointer to the sound to set the channel to. No copy is made of the object, 
so don't delete the JSound objects while using them.

\arg \c current
 Start position in the sound, from which to play.

\arg \c end
 End position, where play should stop, or loop if loop enable.

\arg \c repeat
 Start position of the loop, if loop enable.

\arg \c freq
 Frequency at which you want to play the sound.
 
\arg \c vol
 Volume of the channel. Ranges in 0...255

\arg \c pan
 Panning position of the sound. 0...255. 
(0=left)...(128=middle)...(255=right).

\arg \c loopmode
 See ::LOOPMODE.

\sa Init(), SetSample(), SetFrequency(), SetVolume(), SetPanning()
SetVoiceStatus(), ::LOOPMODE.

\return
Returns \c JOK if no error, or an error received by any other member 
function it calls.
*/
int Mixer::SetVoice(uint32_t ch, StaticSound *sound,
			 uint32_t current, uint32_t end, uint32_t repeat,
			 uint32_t freq, uint8_t vol, uint8_t pan, int loopmode)
{
	PROFILE();
	
	SetVoiceStatus(ch,0);
	SetSample(ch, sound, current, end, repeat, loopmode);
	SetFrequency(ch,freq);
	SetVolume(ch,vol);
	SetPanning(ch,pan);

	return ERR_OK;
}


int Mixer::SetVoice(uint32_t ch, StreamSound *stream, uint8_t vol, bool loop)
{
	PROFILE();

	SetVoiceStatus(ch, 0);
	// Set the StaticSound we'll be using to feed new data from the stream
	SetSample(ch, stream->GetWorkBuffer(), 0, stream->GetWorkBuffer()->GetNumFrames(), 0, SOUND_LOOP_NORMAL);
	SetFrequency(ch, stream->GetFrequency());
	SetVolume(ch, vol);
	SetPanning(ch, AUDIO_PAN_MIDDLE);
	stream->PrepareToPlay(loop);
	// NOTE : Needs to be at the end, because SetSample clears this
	SetMixingListener(ch, stream);

	return ERR_OK;
}


// Set some of the parameters of a channel
/*
 Use this member function to set the sound, and the play positions 
 of a channels.
 
 The parameters are the same as the first ones in SetVoice().

 The channel stays in the same state as before.
If the channel was active, is remains active, if disable, it remains disable.

\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.


\sa Init(), SetVoice(), SetVoiceStatus(),::LOOPMODE
*/
int Mixer::SetSample(int ch, StaticSound *sound,uint32_t current, uint32_t end, uint32_t repeat,  int loopmode)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif	

	CHANNEL *chptr=&channels[ch];

	// save oldstate
	uint8_t oldstate=chptr->on;
	chptr->on=0; // set to disable
	
	chptr->back=0;
	chptr->snd=sound;
	chptr->pos=current<<FREQFRACBITS;
	chptr->end=end<<FREQFRACBITS;
	chptr->repeat=repeat<<FREQFRACBITS;
	chptr->lloop=loopmode;
	chptr->mixingListener = NULL;

	if(sound->Is16Bits())
	{
		if (sound->IsStereo())
			chptr->mixfunc=Mix16StereoFunc;
		else
			chptr->mixfunc=Mix16MonoFunc;
	}
	else
	{
		if (sound->IsStereo())
		{
			chptr->mixfunc=Mix8StereoFunc;
		}
		else
			chptr->mixfunc=Mix8MonoFunc;
	}
	
	chptr->on=oldstate; // return old state

	chptr->volState.restart = true;
	// If the channel is on, then we need to apply a sample stop ramp
	if (chptr->on==1)
		chptr->stopRampState.restart = true;

	return ERR_OK;	
}

// Set a channel's play position.
/*
 Use this member function to set the play position of a channel.

\arg \c ch
 The channel to which to set the play position. 
\arg \c current
 The play position in the sound. 

\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.
 
\sa Init(), SetVoice(), SetSample(), SetVoiceStatus()
*/
int Mixer::SetPosition(int ch, uint32_t current)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif	
	
	CHANNEL *chptr=&channels[ch];	
	chptr->pos=current<<FREQFRACBITS;
	return ERR_OK;
}

// Set the looping positions and the loop mode of a channel.
/*
 Use this member function to set the looping positions, and the loop mode 
of a channel.
 The parameters it uses are the same used in SetVoice() and SetSample().

\arg \c ch
 The channel it applies to.
 
\arg \c loopbeg
 The starting position of the loop.

\arg \c loopend
 The end of the loop.
 
\arg \c loopmode
 See ::LOOPMODE
 
\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.

\sa Init(), SetVoice(), SetSample(), SetVoiceStatus(), ::LOOPMODE
*/
int Mixer::SetLoop(uint8_t ch, uint32_t loopbeg, uint32_t loopend, int loopmode)
{
	PROFILE();

#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	CHANNEL *chptr=&channels[ch];

	uint8_t oldstate=chptr->on;
	chptr->on=0;
	
	chptr->repeat=loopbeg<<FREQFRACBITS;
	chptr->end=loopend<<FREQFRACBITS;
	chptr->lloop=loopmode;
	chptr->back=0;	
	if(chptr->pos>=chptr->end) chptr->pos=chptr->end-(1L<<FREQFRACBITS);	

	chptr->on=oldstate;

	return ERR_OK;
}

// Set the sound frequency of a channel
/*
 Use this member function to set the frequency of a channel.

\arg \c ch
 The channel to use.
\arg \c freq
 The frequency.

\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.

\sa Init(), SetVoice(), SetSample(), SetVoiceStatus()
*/
int Mixer::SetFrequency(int ch, uint32_t freq)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	CHANNEL *chptr=&channels[ch];

	uint8_t oldstate=chptr->on;
	chptr->on=0;
	
	chptr->freq=freq;
	
	chptr->increment=MakeFixed(freq,uint16_t(FREQUENCY));

	chptr->on=oldstate;
	return ERR_OK;
}	

// Set the volume of a channel.
/*
 Use this member function to set the volume of a channel.

\arg \c ch
 The channel to use.
\arg \c vol
 The volume. 0...255

\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.

\sa Init(), SetVoice(), SetVoiceStatus()
*/
int Mixer::SetVolume(int ch, uint8_t vol)
{
	PROFILE();	
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif
	
	channels[ch].vol=vol;
	return ERR_OK;
}

// Set the pan position of a channel
/*
 Use this member function to set the panning position of a channel.

\arg \c ch
 The channel to use.
\arg \c pan
 The panning position. See SetVoice().
 
\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.

\sa Init(), SetVoice(), SetVoiceStatus()
*/
int Mixer::SetPanning(int ch, uint8_t pan)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif
	
	channels[ch].panning=pan;	
	return ERR_OK;
}


// Set a channel ON, or OFF.
/*
 Use this member function to set a channel active, or non-active.

\arg \c ch
 The channel to use.
\arg \c on
 Interpreted as a boolean value. TRUE(non-zero) means active(ON). 
FALSE(0) means non-active(OFF).

\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.
 
\sa Init(), SetVoice()
*/
int Mixer::SetVoiceStatus(int ch, int on)
{
	PROFILE();

#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	uint8_t oldstate = channels[ch].on;
	channels[ch].on=((on)&&(channels[ch].snd!=NULL))? 1: 0;

	if (channels[ch].on && oldstate==0)
		channels[ch].volState.restart = true;
	else if (channels[ch].on==0 && oldstate==1)
		channels[ch].stopRampState.restart = true;

	return ERR_OK;
}


int Mixer::SetMixingListener(int ch, ChannelMixingListener* listener)
{
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	CHANNEL* chPtr = &channels[ch];
	chPtr->mixingListener = listener;
	return ERR_OK;
}


void Mixer::SetMasterVolume(uint8_t v)
{
	PROFILE();
	
	int index=nch;
	v=MIN(64,v);
	CHANNEL *chptr=&channels[0];
	while(index--){
		chptr->mastervol=v;
		chptr++;
		}

//	g_MasterVolume = v;
}

// Set the master volume of a range of channels
/*
 This member functions sets the master volume of some channels.

\arg \c v
 The volume(0...64). Forced to 64 if above 64.
\arg \c firstchannel
 The first channel to set.
\arg \c howmany
 The number of channels to set. If this parameter is greater than the number 
of channels available from \c firstchannel forward, then all all channels from 
\c firstchannel forward are used.

\retval JOK
Ok.
\retval JINVPAR
An invalid parameter was passed.

\sa Init(), SetVoice(), SetVoiceStatus()
*/
int Mixer::SetMasterVolume(uint8_t v,uint32_t firstchannel,uint32_t howmany)
{
	PROFILE();
	
#if CZ_DEBUG
	if((int)firstchannel>=nch) CZERROR(ERR_INVPAR);
#endif


	uint8_t temp = uint8_t(v);

	temp=MIN(64,temp);
	CHANNEL *chptr=&channels[firstchannel];
	if(howmany>nch-firstchannel) howmany=nch-firstchannel;
	while(howmany--){
		chptr->mastervol=temp;
		chptr++;
		}

	return ERR_OK;
}


	

// Check if a channel is ON(active)
/*
 Use this member function to see if a channel is active.

\arg \c ch
 Channel to check.
 
\retval 0
 Channel is OFF(disable) or channel number invalid.
\retval non-zero
 Channel is ON(active)
  
\sa Init(), SetVoice(), SetVoiceStatus()
*/
int Mixer::IsVoiceON(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
    if(ch>=nch) CZERROR(ERR_INVPAR);
#endif
    
//	if(ch>=nch) return 0;
	return channels[ch].on;
};


int Mixer::GetFreeChannel(void){
	PROFILE();

	for (int i=0; i< nch; i++){
		if ((!channels[i].on)&&(channels[i].reserved==0)) return i;
	}

	return -1; // no channel available
}

// Get the master volume of a channel.
/*
 Use this member function to get the master volume of a channel.
 
\arg \c ch
 Channel to use.

\return
 If 0 or above, is the the master volume.
 If negative, then is an error.
 
\sa Init(), SetVoice()
*/
int Mixer::GetMasterVolume(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].mastervol;
};


int Mixer::ReserveChannels(int numChannels)
{
	int maxConsecutiveNumChannels=0;
	int maxConsecutiveFirstChannel=0;

	int tmpNumCh=0; // will hold temporary maximum

	for (int ch=0; ch< nch; ch++){
		if((!channels[ch].on)&&(channels[ch].reserved==0)){
			tmpNumCh++;
		} else {
			// Test if the consecutive available channels
			if (tmpNumCh>maxConsecutiveNumChannels){
				maxConsecutiveNumChannels = tmpNumCh;
				maxConsecutiveFirstChannel = ch - tmpNumCh;
			}
			tmpNumCh = 0;
		}
	}

	// Test again
	if (tmpNumCh>maxConsecutiveNumChannels){
		maxConsecutiveNumChannels = tmpNumCh;
		maxConsecutiveFirstChannel = nch - tmpNumCh;
	}


	if (maxConsecutiveNumChannels<numChannels) CZERROR(ERR_NOTAVAILABLE);

	// Enough consecutive channels found, so mark them as reserved
	for (int i=maxConsecutiveFirstChannel; i<maxConsecutiveFirstChannel+numChannels; i++) channels[i].reserved = 1;
	// return the first channel reserved
	return maxConsecutiveFirstChannel;
}

void Mixer::FreeChannels(int firstChannel, int numChannels)
{
	for (int ch=firstChannel; ch< firstChannel+numChannels; ch++) channels[ch].reserved = 0;
}

int Mixer::ReserveSingleChannel(int channel)
{
	for (int ch=0; ch< nch; ch++){
		if((!channels[ch].on)&&(channels[ch].reserved==0)){
			channels[ch].reserved = 1;
			return ch;
		}
	}

	CZERROR(ERR_NOTAVAILABLE);
}

int Mixer::FreeChannel(int channel){
	channels[channel].reserved = 0;
	return ERR_OK;
}


int Mixer::GetFrameSizeBytes(void)
{
	int ret = 1;
	if (IsStereo()) ret *=2;
	if (Is16Bits()) ret *=2;
	return ret;
}


uint32_t Mixer::GetChannelTag(int ch)
{
		return channels[ch].tag;
}

void Mixer::SetChannelTag(int ch, uint32_t tag)
{
	channels[ch].tag = tag;
}
	
uint32_t Mixer::GetMaxOutputBufferSizeBytes()
{
	uint32_t bytes = samples;
	if (IS_STEREO) bytes *=2;
	if (IS_STEREO) bytes *=2;
	return bytes;
}

uint32_t Mixer::GetMaxOutputBufferSizeFrames()
{
	return samples;	
}

bool Mixer::IsStereo(void)
{
	PROFILE();
	
	return IS_STEREO;
}

bool Mixer::Is16Bits(void)
{
	PROFILE();
	
	return IS_16BITS;
}

int Mixer::GetMixFrequency(void)
{
	PROFILE();
	
	return FREQUENCY;
}

int Mixer::GetChannels(void)
{
	PROFILE();

	return nch;
}


//--------
//
//  EXTRA FUNCTIONS
//
//--------

#if CZ_PLAYER_EXTRAFUNCTIONS_ENABLED



// Get the current playing position of a channel
/*
 Use this member function to get the current playing position of a channel.
 
\arg \c ch
 Channel to use.

\return
 If 0 or above, the current playing position.
 If negative, is an error.

\sa Init(), SetVoice(), SetVoiceStatus()
*/
int  Mixer::GetPosition(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].pos>>FREQFRACBITS;
};

// Get the end position of a channel
/*
 Use this member function to get the end position of a channel.
 
\arg \c ch
 Channel to use.

\return
 If 0 or above, the end position.
 If negative, an error.

\sa Init(), SetVoice(), SetVoiceStatus()
*/
int  Mixer::GetEnd(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].end>>FREQFRACBITS;
};

// Get the repeat position of a channel
/*
 Use this member function to get the start of the loop of a channel.

\arg \c ch
 Channel to use
 
\return
 If 0 or above, the repeat position.
 If negative, an error.

\warning
 Remember that even if looping is not active, there is a repeat position 
(which isn't used).
 
\sa Init(), SetVoice(), SetVoiceStatus()
*/
int  Mixer::GetRepeat(int ch)
{
	PROFILE();
		
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].repeat>>FREQFRACBITS;
};


/*
int Mixer::GetLoopMode(int ch,LOOPMODE *lm)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	*lm=channels[ch].loop;
	return ERR_OK;
};
*/

// Get the frequency of a channel
/*
 Use this member function to get current frequency of a channel.
 
\arg \c ch
 Channel to use.

\return
 If 0 or above, the frequency.
 If negative, an error.
 
\sa Init(), SetVoice(), SetVoiceStatus()
*/
int Mixer::GetFrequency(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].freq;
};

// Get the volume of a channel
/*
 Use this member function to get the volume of a channel.

\arg \c ch
 Channel to use
 
\return
 If 0 or above, the volume.
 If negative, an error.

\sa Init(), SetVoice(), SetVolume(), SetMasterVolume()
*/
int Mixer::GetVolume(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].vol;
};

// Get the pannning of a channel
/*
 Use this member function to get the panning of a channel.

\arg \c ch
 Channel to use
 
\return
 If 0 or above, the panning.
 If negative, an error.

\sa Init(), SetVoice(), SetPanning()
*/
int Mixer::GetPanning(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	return channels[ch].panning;
};


// Return a channel's current value.
/*
 Use this member function to get the current value of a channel.

i.e: Imagine the sound wave. That sound is being played in a channel, which 
has a variable which points to the wave sample being played.
This member function gets the 'weight' of that sample.

\arg \c ch
The channel to use. If the channel is not playing, then 0 is returned 
(no 'weight') to the final output.

\return
 If the return value is greater than or equal to 0, then its the channel 
value. That value uses 14 bits, so is in the range 0...16383.
 If the return value is negative, then is an error.

\sa Init(), SetVoice(), SetVoiceStatus()
*/
int Mixer::GetVoiceValue(int ch)
{
	PROFILE();
	
#if CZ_DEBUG
	if(ch>=nch) CZERROR(ERR_INVPAR);
#endif

	CHANNEL *chptr=&channels[ch];
	if(!chptr->on) return 0;
	StaticSound *snd=chptr->snd;

	uint32_t vol=(uint32_t)chptr->mastervol*(uint32_t)chptr->vol;
	void *ptr=snd->GetPtrToFrame(chptr->pos>>FREQFRACBITS);	
	if(snd->Is16Bits()){
		uint16_t s=*(uint16_t *)ptr+32768;
		uint32_t value=((uint32_t)s*vol) >> 16;
		return (value);
		}
		else
		{
		uint8_t s=*(uint8_t *)ptr+128;
		uint32_t value=((uint32_t)s*vol) >> 8;
		return (value);
		}
}



// Check if a channel is playing backwards
/*
 When a loop is set to ::LOOP_PINGPONG, the sound will eventualy play 
backwards when it reachs the looping position.
 
 This member function tells if a channel is playing backwards.
 
\arg \c ch
 Channel to check.

\retval 0
 The channel is not playing backwards, or channel number invalid.

\retval non-zero
 The channel is playing backwards.

\sa Init(), SetVoice(), SetLoop()
*/
int Mixer::IsVoiceGoingBack(int ch)
{
	PROFILE();
	
	if(ch>=nch) return 0;
	return channels[ch].back;
};


#endif // CZ_PLAYER_EXTRAFUNCTIONS_ENABLED

} // namespace audio
} // namespace cz
