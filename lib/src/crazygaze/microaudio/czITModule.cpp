//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czITModule.h"
#include "czStaticSound.h"
#include "czMixer.h"
#include "czFile.h"
#include "czStandardC.h"

//
// Wrap the entire source code.
//

#if CZ_PLAYER_IT_ENABLED




//#define PROFILE() PROFILE_OFF

#define DATACHECK 

#define CLEARARRAY(a,n) memset(a, 0, sizeof(a[0])*(n))

#define inrange(val,low,high)	((val>high)?0:((val<low)?0:1))


#define ENTER_CRITICAL
#define LEAVE_CRITICAL

namespace cz
{
namespace microaudio
{


// Multiply an unsigned FixedPoint by an unsigned Integer, and return an integer
// Or
// Multiply a unsigned fixed by a unsigned fixed, and return a unsigned fixed
inline unsigned int fixed_X_int(uint32_t p1,uint32_t p2)
{
	TInt64 v(TInt64(p1)*TInt64(p2));
	TInt64_Lsr(v,16);
	unsigned int ret = (unsigned int) TInt64_GetTInt(v);
	//return (unsigned int)(((uint64_t)((uint64_t)(p1) * (uint64_t)p2)) >> 16);
	return ret;
}

/*
int AmigaFreqSlide(int oldFreq, int targetFreq, int dat)
{
	const k=7093789.2f;
	float f = oldFreq;
	float d = (targetFreq<oldFreq) ? dat : -dat;	
	float nf = k / ((k/(f*2) + d)*2);
	return (int) nf;
}
*/


inline fixed16_16 FMUL16_16(fixed16_16 a, fixed16_16 b){
	TInt64 v ( (TInt64(a) * TInt64(b)) );
	TInt64_Lsr(v,16);
	return (fixed16_16) TInt64_GetTInt(v);
}
inline fixed16_16 FDIV16_16(fixed16_16 a, fixed16_16 b){
	TInt64 v( (TInt64(a) * TInt64(65536)) / TInt64(b));
	return (fixed16_16) TInt64_GetTInt(v);
}

// Mophun 18.14 fixed point
typedef int32_t fixed32_t;
fixed32_t vPow(fixed32_t val, uint8_t exp){
	if (exp==0) {
		return (1<<14);
	} else if (exp==1){
		return val;
	} else {
		float ret = pow(float(val)/(2^Mixer::FREQFRACBITS), float(exp) );
		return (fixed32_t) (ret*(2^Mixer::FREQFRACBITS)) ;
	}
}

/*
#define FMUL16_16(a,b) ((fixed16_16) ( ((int64_t)(a) * (int64_t)(b)) >> 16))
#define FDIV16_16(a,b) ((fixed16_16) ( ((int64_t)(a) << 16) / (b)))
*/
#define FIX16_16(a) ((fixed16_16)(((fixed16_16)(a))<<16))
#define FIXTOI16_16(a) ((int)(((fixed16_16)(a))>>16))
#define FIXXX16_16(a) (a)

#define NNA_CUT 0
#define NNA_CON 1
#define NNA_OFF 2
#define NNA_FAD 3

#define DCT_OFF 0
#define DCT_NTE 1
#define DCT_SMP 2
#define DCT_INS 3

#define DCA_CUT 0
#define DCA_OFF 1
#define DCA_FAD 2


/*
int ITModule::FeedData(void *ptr, int bytes)
{
	PROFILE();

	
	// Calculate num of samples
	int len=(bytes*8)/m_samplesize;
	int todo=len;
	int part,pos=0;
	uint8_t *mx=(uint8_t *)ptr;

	int FREQUENCY = m_mixer->GetMixFrequency();

	//CZLOG("FeedData %d samples\n", len);
	
	while(todo>0){
		if (m_reachedEnd){
			int done = ((len-todo)*m_samplesize)/8;
			int left = bytes - done;
			//CZLOG("****SONG END. BYTES REQUEST=%d DONE=%d \n", bytes, done);
			// Silence the rest of the block
			memset(&mx[pos], (Is16Bits()) ? 0:128 , left);
			return done;
		}

		if (BPMMixToDo<=0){
			ItPlayer();
			BPMMixToDo=(125L*FREQUENCY)/(50L*m_bpm);
		}		
				

		part=MIN(BPMMixToDo,todo);

		m_mixer->MixPortion(&mx[pos],part);

//		raw->WriteData(&mx[pos], part*2);
		BPMMixToDo-=part;
		todo-=part;
		pos+=(part*m_samplesize)/8;
	}
	
	// Return number of bytes mixed
	return((len*m_samplesize)/8);	
}
*/

void ITModule::CleanUpMemory(void)
{	
	PROFILE();
	
	if (patterns!=NULL){
		int count;
		for (count=0;count< it_header.PatNum;count++)
			if(patterns[count].data) CZFREE(patterns[count].data);
		CZFREE(patterns);
	}

		
	if (ChLastInfo!=NULL) CZFREE(ChLastInfo);
	if (rowinfo!=NULL) CZFREE(rowinfo);
	if (track!=NULL) CZFREE(track);
	
	if (samples!=NULL) CZFREE(samples);
	if (waves!=NULL) {
		for (int i=0; i< it_header.SmpNum; i++){
			if(waves[i]!=NULL) CZDELETE(waves[i]);
		}
		CZFREE(waves);
	}
	if (instruments!=NULL) CZFREE(instruments);
	if (Orders!=NULL) CZFREE(Orders);
	
	if (virtualchannels!=NULL) CZFREE(virtualchannels);

}

void ITModule::ResetMembers(void)
{
	PROFILE();
	
	m_bpm=125;
	
	// last one to reset, because some values are needed to reset the others
	memset(&it_header,0,sizeof it_header);

	patterns = NULL;
	samples = NULL;
	waves = NULL;
	instruments = NULL;
	Orders = NULL;
	virtualchannels = NULL;

	ChLastInfo = NULL;
	rowinfo = NULL;
	track = NULL;

	Loaded=0;
	IsPaused=0;	
	m_reachedEnd = false;
	m_numVirtualChannels=0;
	IsPlaying=0;
	
}

/*******************************************************************
 *******************************************************************
 ********                       INTERFACE                   ********
 *******************************************************************
 *******************************************************************/


//! Constructor
/*!
 The constructor simply initializes some variables.
 
\sa Init()
*/
ITModule::ITModule(::cz::Core *parentObject) : ::cz::microaudio::Module(parentObject)
{
	PROFILE();

	ResetMembers();
}

//! Destructor
/*!
 The destructor frees any allocated memory.
*/
ITModule::~ITModule()
{
	PROFILE();

	CleanUpMemory();		
}	



int ITModule::ReadHeader(::cz::io::File *in)
{
	PROFILE();
	
	in->ReadData(&it_header.id[0], 4);
	if ((memcmp("IMPM", it_header.id, 4)))
	{
		CZERROR(ERR_WRONGFORMAT);
	}

	in->ReadData(&it_header.modname[0],26);
	in->ReadData(&it_header.notused[0], 2);
	it_header.OrdNum = in->ReadUnsigned16();
	it_header.InsNum = in->ReadUnsigned16();
	it_header.SmpNum = in->ReadUnsigned16();
	it_header.PatNum = in->ReadUnsigned16();
	it_header.CwtV   = in->ReadUnsigned16();
	it_header.Cmwt   = in->ReadUnsigned16();
	it_header.Flags  = in->ReadUnsigned16();
	it_header.Special= in->ReadUnsigned16();
	it_header.GV  = in->ReadUnsigned8();
	it_header.MV  = in->ReadUnsigned8();
    //CZLOG("\n\nMV=%d\n\n", (int)it_header.MV);
	it_header.IS  = in->ReadUnsigned8();
	it_header.IT  = in->ReadUnsigned8();
	it_header.Sep = in->ReadUnsigned8();
	it_header.value0 = in->ReadSigned8();	
	it_header.MsgLength = in->ReadUnsigned16();
	it_header.MsgOffset = in->ReadSigned32();
	in->ReadData(&it_header.reserved[0], 4);
	in->ReadData(&it_header.ChnlPan[0], 64);
	in->ReadData(&it_header.ChnlVol[0], 64);

	CZLOG(LOG_INFO, "ModName=%s\n", it_header.modname);
	CZLOG(LOG_INFO, "OrdNum=%d ", it_header.OrdNum);
	CZLOG(LOG_INFO, "InsNum=%d ", it_header.InsNum);
	CZLOG(LOG_INFO, "SmpNum=%d ", it_header.SmpNum);
	CZLOG(LOG_INFO, "PatNum=%d\n", it_header.PatNum);

	return ERR_OK;	
}


int ITModule::ReadEnvelope(::cz::io::File *in, IT_ENVELOPE *env)
{
	PROFILE();
	
	env->Flg = in->ReadUnsigned8();
	env->Num = in->ReadUnsigned8();
	env->LpB = in->ReadUnsigned8();
	env->LpE = in->ReadUnsigned8();
	env->SLB = in->ReadUnsigned8();
	env->SLE = in->ReadUnsigned8();
	for (int n=0; n<25; n++){
		env->Nodes[n].y = in->ReadSigned8();
		env->Nodes[n].tick = in->ReadUnsigned16();
	}
	env->unused = in->ReadSigned8();
	
	return ERR_OK;	
}

int ITModule::ReadInstruments(::cz::io::File *in)
{
	PROFILE();
	
	
#if CZ_DEBUG
	CZLOG(LOG_INFO, "Reading Instruments...\n");
	CZLOG(LOG_INFO, "Number of instruments = %d\n", (int)it_header.InsNum);
	if (it_header.InsNum>MAX_IT_INSTRUMENTS) CZERROR(ERR_WRONGFORMAT);
#endif

	int err;
	
	// read instrument offsets
	if ((err=in->Seek(0xC0+it_header.OrdNum, ::cz::io::FILE_SEEK_START))!=ERR_OK) CZERROR(err);
	if ((err=in->ReadData(dumptr, (int)it_header.InsNum*4))!=ERR_OK) CZERROR(err);

	// Allocate memory for instruments
	instruments = (IT_INSTRUMENT*) CZALLOC(sizeof(IT_INSTRUMENT)*it_header.InsNum);
	if (instruments==NULL) CZERROR(ERR_NOMEM);
	CLEARARRAY(instruments, it_header.InsNum);
			
	// Read instruments
	for(int count=0;count<it_header.InsNum;count++)
	{
		IT_INSTRUMENT *inst = &instruments[count];
		
		if((err=in->Seek(dumptr[count], ::cz::io::FILE_SEEK_START))!=ERR_OK) CZERROR(err);
		
		in->ReadData(&inst->ID[0],4);
		in->ReadData(&inst->FileName[0],12);
		inst->value_00h = in->ReadUnsigned8();		
		inst->NNA = in->ReadUnsigned8();
		inst->DCT = in->ReadUnsigned8();
		inst->DCA = in->ReadUnsigned8();
		inst->FadeOut = in->ReadUnsigned16();
		inst->PPS = in->ReadSigned8();

		inst->PPC = in->ReadUnsigned8();
		inst->GbV = in->ReadUnsigned8();
		inst->DfP = in->ReadUnsigned8();
		inst->RV = in->ReadUnsigned8();
		inst->RP = in->ReadUnsigned8();
		inst->TrkVers = in->ReadUnsigned16();
		inst->NoS = in->ReadUnsigned8();
		inst->unused = in->ReadSigned8();
		in->ReadData(&inst->InsName[0], 26);
		in->ReadData(&inst->notused[0], 2);
		inst->MCh = in->ReadUnsigned8();
		inst->MPr = in->ReadUnsigned8();
		inst->MIDIBnk = in->ReadUnsigned16();
		for(int k=0; k<120; k++){
			inst->KybdTbl[k].note = in->ReadUnsigned8();	
			inst->KybdTbl[k].sample = in->ReadUnsigned8();	
		}
		
		ReadEnvelope(in, &inst->VolEnv);
		ReadEnvelope(in, &inst->PanEnv);
		ReadEnvelope(in, &inst->PitEnv);		
	}	

	return ERR_OK;	
}


int ITModule::ReadSamples(::cz::io::File *in)
{
	PROFILE();

	
#if CZ_DEBUG
	CZLOG(LOG_INFO, "Reading Samples...\n");
	CZLOG(LOG_INFO, "Number of samples = %d\n", (int)it_header.SmpNum);	
	if (it_header.SmpNum > MAX_IT_SAMPLES) CZERROR(ERR_WRONGFORMAT);
#endif

		
	IT_SAMPLE *smp;
	StaticSound *wv;
	int err;

	
	int count;	
	// read samples offsets
	count=0xC0+(int)it_header.OrdNum+(int)it_header.InsNum*4;
	if ((err=in->Seek(count, ::cz::io::FILE_SEEK_START))!=ERR_OK) goto ERROR_EXIT;
	if ((err=in->ReadData(dumptr, (int)it_header.SmpNum*4))!=ERR_OK) goto ERROR_EXIT;

	// Allocate memory for samples
	samples = (IT_SAMPLE*) CZALLOC(sizeof(IT_SAMPLE)*it_header.SmpNum);
	if (samples==NULL) CZERROR(ERR_NOMEM);
	CLEARARRAY(samples, it_header.SmpNum);
	
		
	// Read samples
	for(count=0;count<it_header.SmpNum;count++)
	{
		if((err=in->Seek(dumptr[count], ::cz::io::FILE_SEEK_START))!=ERR_OK) goto ERROR_EXIT;
		IT_SAMPLE *smp = &samples[count];
				
		in->ReadData(&smp->ID[0], 4);
		in->ReadData(&smp->FileName[0], 12);
		smp->value_00h = in->ReadUnsigned8();
		smp->GvL = in->ReadUnsigned8();								
		smp->Flg = in->ReadUnsigned8();								
		smp->Vol = in->ReadUnsigned8();								
		in->ReadData(&smp->SmpName[0],26);				
		smp->Cvt = in->ReadUnsigned8();
		smp->DfP = in->ReadUnsigned8();
		smp->Length  = in->ReadSigned32();
		smp->LoopBeg = in->ReadSigned32();
		smp->LoopEnd = in->ReadSigned32();		
		smp->C5Speed = in->ReadSigned32();
		smp->SusLBeg = in->ReadSigned32();
		smp->SusLEnd = in->ReadSigned32();
		
		smp->SmpPtr = in->ReadUnsigned32();
		
		smp->ViS = in->ReadUnsigned8();				
		smp->ViD = in->ReadUnsigned8();				
		smp->ViR = in->ReadUnsigned8();				
		smp->ViT = in->ReadUnsigned8();						
	}	
	
	// Allocate memory for our own sound objects
	waves = (StaticSound**) CZALLOC(sizeof(StaticSound*)*it_header.SmpNum);
	memset(waves, 0, sizeof(StaticSound*) * it_header.SmpNum);
	if (waves==NULL) CZERROR(ERR_NOMEM);

	for(int i=0;i<it_header.SmpNum;i++)
	{
		smp=&samples[i];
		if (smp->Flg&SMP_IT_COMPRESSED)
		{
			CZLOG(LOG_ERROR, "IT 2.14 compressed samples not supported.");
			CZERROR(ERR_WRONGFORMAT);
		}

		if ((smp->Flg&SMP_IT_SAMPLE)&&(smp->Length!=0)) // sample associated
		{
			// Only allocate StaticSound object when there is a sample associated
			waves[i] = CZNEW(StaticSound) (m_core);
			if (waves[i]==NULL) CZERROR(ERR_NOMEM);
			wv = waves[i];
		
			// Used when decompressing a ADPCM sample
			int8_t *uncompressedData=NULL;
		
			// Move to the sample data position first, because ADPCM format needs to read data
			if((err=in->Seek(smp->SmpPtr, ::cz::io::FILE_SEEK_START))!=ERR_OK)
				goto ERROR_EXIT;			
			
			int format=SOUND_LOOP_OFF|SOUND_MONO;		
			int lenBytes=smp->Length;
			bool is_16=false;
	        if(smp->Flg&SMP_IT_16BIT){
				lenBytes *=2;
				is_16=1;
			};
			switch (smp->Cvt){	// check sample type
				case 1: 	// signed
					if(is_16) format|=SOUND_16BITS|SOUND_SIGNED;
					else format|=SOUND_8BITS|SOUND_SIGNED;
					break;
				case 0:		// unsigned
					if(is_16) format |= SOUND_16BITS|SOUND_UNSIGNED;
					else format|= SOUND_8BITS|SOUND_UNSIGNED;
					break;
				case 255: // 4-Bit ADPCM
					{
						CZLOG(LOG_INFO, "Reading 4-Bit ADPCM.\n");
						format |= SOUND_8BITS|SOUND_SIGNED;					
						int8_t compressionTable[16];
						int adpcmLen = (smp->Length + 1) / 2;
						// Read compression table at the beginning of sample
						if((err=in->ReadData(compressionTable,16))!=ERR_OK)
							goto ERROR_EXIT;
							
						// Allocate memory to uncompressed the sample
						// add 1 extra byte, because odd lengths will need 1 extra byte
						uncompressedData = (int8_t*) CZALLOC(sizeof(int8_t)*(smp->Length+1));
						if (uncompressedData==NULL) CZERROR(ERR_NOMEM);
						// Allocate memory to read compressed data from file, to speed things up
						uint8_t *compressedData = (uint8_t*) CZALLOC(sizeof(uint8_t)*adpcmLen);
						if (compressedData==NULL) CZERROR(ERR_NOMEM);
					
						int8_t *sptr = uncompressedData;
						uint8_t *compressedPtr = compressedData;
						int8_t delta = 0;
						//CZLOG("Bytes to read from file = %d\n", adpcmLen);
						// Read the compressed data into the temporary buffer
						if((err=in->ReadData(compressedData,adpcmLen))!=ERR_OK)
							goto ERROR_EXIT;					
						for (int32_t j=0; j<adpcmLen; j++)
						{
							uint8_t s = *compressedPtr;
							compressedPtr++;
							uint8_t b0 = s & 0x0F;
							uint8_t b1 = s  >> 4;
							delta = (int8_t)(delta + compressionTable[b0]);
							sptr[0] = delta;
							delta = (int8_t)(delta + compressionTable[b1]);
							sptr[1] = delta;
							sptr += 2;
						}
						// Free the temporary buffer used to read the compressed data
						CZFREE(compressedData);
					}
					break;
				default:
					CZLOG(LOG_INFO, "Invalid sample format\n");
					//User::Panic(_L("Invalid sample format"),3); 
					exit(EXIT_FAILURE);
					//vTerminateVMGP();
					break;
			}
			
			CZLOG(LOG_INFO, "Reading sample %d, C5Speed=%d, size (frames)=%d\n", i, smp->C5Speed, smp->Length);
			if((err=wv->Set(format,smp->Length))!=ERR_OK)
				goto ERROR_EXIT;
			if (uncompressedData!=NULL){
				// Its a APCM sample, and is already in memory
				memcpy(wv->GetPtrToFrame(0), uncompressedData, lenBytes);
				CZFREE(uncompressedData);
			} else {
				// Normal sample
				if((err=in->ReadData(wv->GetPtrToFrame(0),lenBytes))!=ERR_OK)
					goto ERROR_EXIT;

				/*
				czSymbianFile out(m_mainObj);
				out.Open(_L("sample.raw"), CZFILE_WRITE);
				out.WriteData(wv->GetPtrToSample(0), len);
				out.Close();
				*/
			}

			if(!wv->IsSigned()) wv->ChangeSign();
		
			//wv->ExpandBoundSamples();
			int repeat=0;
			int end=wv->GetNumFrames();
			int lmode=SOUND_LOOP_OFF;

			/*
			if(smp->Flg&SMP_IT_SUSTAINLOOP)
				if(!(smp->Flg&SMP_IT_PINGPONGSUSTAINLOOP))
					if(smp->SusLEnd==wv->GetFrames())
					{
						lmode=LOOP_NORMAL;
						repeat=smp->SusLBeg;
						end=smp->SusLEnd;
					}
			if(smp->Flg&SMP_IT_LOOP)
				if(!(smp->Flg&SMP_IT_PINGPONGLOOP))
					{
					if((lmode==LOOP_OFF)
						||((lmode==LOOP_NORMAL)&&(end<smp->LoopEnd)))
						{
						lmode=LOOP_NORMAL;
						repeat=smp->LoopBeg;
						end=smp->LoopEnd;
						}
					}
			//if(lmode==LOOP_NORMAL) wv->SmoothLoop(repeat,end);
			*/

			// Only consider to smooth a loop if there is no sustain loop, or the sustain loop end  is before the normal loop end
			if (smp->Flg&SMP_IT_LOOP)
				if ( (!(smp->Flg&SMP_IT_SUSTAINLOOP)) ||
					 ((smp->Flg&SMP_IT_SUSTAINLOOP)&&(smp->SusLEnd<smp->LoopEnd))   )
				{
					lmode = (smp->Flg&SMP_IT_PINGPONGLOOP) ? SOUND_LOOP_BIDI : SOUND_LOOP_NORMAL;
					repeat = smp->LoopBeg;
					end = smp->LoopEnd;
				}
			wv->SetLoopMode(lmode, repeat, end-repeat);
			wv->SmoothLoop();
		}
	}

return ERR_OK;

	ERROR_EXIT:
	CZERROR(err);
}



int ITModule::ReadPatterns(::cz::io::File *in)
{
	PROFILE();
	
#if CZ_DEBUG
	CZLOG(LOG_INFO, "Reading patterns...\n");
	CZLOG(LOG_INFO, "Number of patterns = %d\n", (int)it_header.PatNum);
	if (it_header.PatNum>MAX_IT_PATTERNS) CZERROR(ERR_WRONGFORMAT);
#endif
	
	int err=0;
	int count;	
	// read patterns offsets
	count=0xC0+(int)it_header.OrdNum+(int)it_header.InsNum*4+(int)it_header.SmpNum*4;
	if ((err=in->Seek(count, ::cz::io::FILE_SEEK_START))!=ERR_OK) goto ERROR_EXIT;
	if ((err=in->ReadData(dumptr, (int)it_header.PatNum*4))!=ERR_OK) goto ERROR_EXIT;

	// Allocate memory for patterns's array
	patterns = (IT_PATTERN*) CZALLOC(sizeof(IT_PATTERN)*it_header.PatNum);
	if (patterns==NULL) CZERROR(ERR_NOMEM);
	CLEARARRAY(patterns, it_header.PatNum);

	// Read patterns
	for(count=0;count<it_header.PatNum;count++)
	{
		IT_PATTERN *pat=&patterns[count];
		if (dumptr[count]==0){
			pat->Length=0;
			pat->Rows=64;
			pat->data=NULL;
		}else{
			if ((err=in->Seek(dumptr[count], ::cz::io::FILE_SEEK_START))!=ERR_OK) goto ERROR_EXIT;
			
			pat->Length = in->ReadUnsigned16();
			pat->Rows   = in->ReadUnsigned16();
			
			// Read Unused
			in->ReadUnsigned32();
			
			// read pattern data		
			int len=pat->Length;
			if(len!=0){
				pat->data=(unsigned char *) CZALLOC(sizeof(char)*len);
				if(pat->data==NULL){
					err=ERR_NOMEM;
					goto ERROR_EXIT;
				}
				if ((err=in->ReadData(pat->data,len))!=ERR_OK) goto ERROR_EXIT;
			}			
		}
		
	}

	return ERR_OK;

	ERROR_EXIT:
	CZERROR(err);
}

int ITModule::Start(Mixer *mixer, int firstOrder, int lastOrder, bool loop, uint8_t volume){

	PROFILE();

	if ((!Loaded)||(IsPlaying)) CZERROR(ERR_CANTRUN);
	
	// Virtual channels are only allocated here, because only here we have the m_mixer object available
	// Allocate only if needed (no other previous and suitable allocation made)
	if (m_numVirtualChannels<mixer->GetChannels()){
		// Allocate virtual channels
		m_numVirtualChannels=mixer->GetChannels();
		// Allocate memory for the virtual channels
		if (virtualchannels!=NULL) CZFREE(virtualchannels);
		virtualchannels = (VIRTUALCHANNEL*) CZALLOC(sizeof(VIRTUALCHANNEL)*m_numVirtualChannels);
		if (virtualchannels==NULL) CZERROR(ERR_NOMEM);	
		CLEARARRAY(virtualchannels, m_numVirtualChannels);
	}

//	if (mixer->GetChannels() < m_numVirtualChannels) CZERROR(ERR_INVPAR);

//	m_firstMixerChannel=0;

	m_mixer = mixer;

	// Our Mixing volume is 0..64, but IT is 0..128
	m_masterVolume = it_header.MV>>1;
	//	m_mixer->SetMasterVolume((int)it_header.MV>>1);

	m_loop = loop;
	// If is not playing, then set to play all song
	if (!IsPlaying){
		// By default, set to play all
		SetPlayRange(firstOrder, lastOrder);
	}

	return ERR_OK;
}


bool ITModule::CheckFormat(::cz::io::File *in)
{
	int pos = in->GetPos();
	in->Seek(0, ::cz::io::FILE_SEEK_START);
	char buf[4];
	in->ReadData(buf, 4);
	in->Seek(pos, ::cz::io::FILE_SEEK_START);
	return (memcmp("IMPM", buf, 4)==0) ? true : false;
}


int ITModule::Init(::cz::io::File *in)
{	
	PROFILE();
	
	int err;

	if(IsPlaying) CZERROR(ERR_CANTRUN);

	CleanUpMemory();
	ResetMembers();
        
	if ((err=ReadHeader(in))!=ERR_OK) goto ERROR_EXIT;
	
	// Read ORDER
	if ((err=in->Seek(0xC0, ::cz::io::FILE_SEEK_START))!=ERR_OK) goto ERROR_EXIT;	
#if CZ_DEBUG
	CZLOG(LOG_INFO, "Number of orders = %d\n", (int)it_header.OrdNum);
#endif	
	// Allocate memory for orders
	Orders = (unsigned char*) CZALLOC(sizeof(unsigned char)*it_header.OrdNum);
	if (Orders == NULL) CZERROR(ERR_NOMEM);
	// Read from file
	if ((err=in->ReadData(&Orders[0], it_header.OrdNum))!=ERR_OK) goto ERROR_EXIT;               

		
	/*
	 * INSTRUMENT RELATED STUFF
	 */
	// read instrument offsets
	if (it_header.InsNum==0) goto NO_INSTRUMENTS_TO_READ;
	if ((err=ReadInstruments(in))!=ERR_OK) goto ERROR_EXIT;

NO_INSTRUMENTS_TO_READ:

	// Read Samples
	if ((err=ReadSamples(in))!=ERR_OK) goto ERROR_EXIT;

	// Read Patterns
	if ((err=ReadPatterns(in))!=ERR_OK) goto ERROR_EXIT;
	
	if (it_header.Flags&4)
		InstrumentMode=1;
		else InstrumentMode=0;
	if (it_header.Flags&8)
		Linear=1;
		else Linear=0;
	if (it_header.Flags&16)
		OldEffects=1;
		else OldEffects=0;
	if (it_header.Flags&32)
		GxxComp=1;
		else GxxComp=0;
				

	// Count channels
	it_channels=0;
	for (it_channels=0;it_channels<64;it_channels++)
	{
		if(it_channels==64) break;
        if ((it_header.ChnlPan[it_channels]>100)&&(it_header.ChnlPan[it_channels]!=255)){
            CZLOG(LOG_INFO, "ERROR: IT file has surround channels, or disabled channels.\n");
			//User::Panic(_L("Unsupported channel types"), 3);
			exit(EXIT_FAILURE);
        }
        if (it_header.ChnlPan[it_channels]==255) break;
	}
	CZLOG(LOG_INFO, "CHANNELS=%d\n", it_channels);

	// Alloc arrays of size [it_channels]
	ChLastInfo = (IT_CHANNEL_LAST_INFO*) CZALLOC(sizeof(IT_CHANNEL_LAST_INFO)*it_channels);	
	rowinfo = (IT_NOTE*) CZALLOC(sizeof(IT_NOTE)*it_channels);
	track = (TRACK*) CZALLOC(sizeof(TRACK)*it_channels);	
	if ((ChLastInfo==NULL)||(rowinfo==NULL)||(track==NULL)) CZERROR(ERR_NOMEM);
	
	CLEARARRAY(ChLastInfo, it_channels);
	CLEARARRAY(rowinfo, it_channels);
	CLEARARRAY(track, it_channels);
		

// Debug
#if CZ_DEBUG
	for (int i=0; i<it_header.InsNum; i++){
		IT_INSTRUMENT *inst = &instruments[i];
		CZLOG(LOG_INFO, "Instrument %d = %s\n", i, inst->InsName);
		int n=0;
		for (n=0; n< inst->VolEnv.Num; n++)
			CZLOG(LOG_INFO, "VolEnv.Node[%d]={%d,%d}\n", n, (int)inst->VolEnv.Nodes[n].y, (int)inst->VolEnv.Nodes[n].tick);
		for (n=0; n< inst->PanEnv.Num; n++)
			CZLOG(LOG_INFO, "PanEnv.Node[%d]={%d,%d}\n", n, (int)inst->PanEnv.Nodes[n].y, (int)inst->PanEnv.Nodes[n].tick);
	}
#endif // CZ_DEBUG
	
	
    
    // Set play range to all
    Loaded=1;
    CZLOG(LOG_INFO, "IT File loaded with sucess.\n");
    return ERR_OK;
    
    ERROR_EXIT:
    CZERROR(err);
}

int ITModule::SetPlayRange(int firstOrder, int lastOrder)
{
	PROFILE();
	
#if CZ_DEBUG
	if (!Loaded) CZERROR(ERR_CANTRUN);
//	if ((nch<1)||(nch>m_mixer->GetChannels())) CZERROR(ERR_INVPAR);
    if (virtualchannels==NULL) CZERROR(ERR_CANTRUN);
#endif
    if (IsPlaying){
        Stop();
    }

	int ch;
	
    m_firstOrder = ((firstOrder<0)||(firstOrder>it_header.OrdNum-1)) ? 0 : firstOrder;
    m_lastOrder = (lastOrder<0) ? (it_header.OrdNum-1) : lastOrder;
	
	ProcessRow=-1;	
	TickCounter=1;
	RowCounter=1;
	Speed=it_header.IS;
	ProcessOrder=m_firstOrder;
	CurrentPattern=Orders[ProcessOrder];
	NumberOfRows=patterns[CurrentPattern].Rows;
	BreakRow=0;
	CurrentRow=0;
	GlobalVol=it_header.GV;
	FirstTick=0;
	NextPackPos=0;
	PatternDelay=0;
	PatternEnd=0;

	CLEARARRAY(virtualchannels, m_numVirtualChannels);	
	CLEARARRAY(track, it_channels);

	for (ch = 0 ;ch<it_channels ;ch++){
		track[ch].ChannelVol = it_header.ChnlVol[ch];
		track[ch].ChannelPan = it_header.ChnlPan[ch];
	}
	for(ch=0;ch<m_numVirtualChannels;ch++){
		KillChannel(ch);
	}        
	
	//czSoundDevice::SetTempo(it_header.IT);
	m_bpm = it_header.IT;
	
	IsPlaying=1;	
	m_reachedEnd = false;
	if(IsPaused){ // reinit pause
		IsPaused=0;
		Pause();
	} else {
		//czSoundDevice::SetPlayer(this);
	}

	return ERR_OK;
}

int ITModule::Stop(void)
{
	PROFILE();	

	//if((!IsPlaying)||(!Loaded)) CZERROR(ERR_CANTRUN);
    if((!IsPlaying)||(!Loaded)) return ERR_OK;

	int count;
	//czSoundDevice::SetPlayer(NULL);
	for (count =0;count<m_numVirtualChannels;count++){
		KillChannel(count);
	}
	IsPlaying=0;   
	return ERR_OK;
}

int ITModule::Pause(void)
{
	PROFILE();

	if (!Loaded) CZERROR(ERR_OK);

	for (int c=0; c<m_numVirtualChannels;c++){
		if (virtualchannels[c].active){
			if(m_mixer->IsVoiceON(c)){
				virtualchannels[c].paused=true;
				m_mixer->SetVoiceStatus(c,0);
			}else{
				virtualchannels[c].paused=false;
			}
		}
	}
	IsPaused=1;

	return ERR_OK;
}       	

int ITModule::Resume(void){
	if (!IsPaused) return ERR_OK;	

	for(int c=0;c<m_numVirtualChannels;c++){
		if ((virtualchannels[c].active)&&(virtualchannels[c].paused)){
			m_mixer->SetVoiceStatus(c,1);
			virtualchannels[c].paused=false;
		}
	}
	IsPaused=0;

	return ERR_OK;

}
/***************************************************************************
  -------------------------------------------------------------------------
			IT PLAYER ROUTINES
	
              ����� �       ��   �     � ����� �����
              �   � �      �  �   �   �  �     �   �
              ����� �     �    �   � �   ���   �����
              �     �     ������    �    �     � �  
              �     ����� �    �    �    ����� �  ��

  -------------------------------------------------------------------------
 ***************************************************************************/

/***************************************************************************
			GET ROW
  Fills row[]
  returns:
  	position where it left						
 ***************************************************************************/
 
int ITModule::GetITRow(int patnumber,int pos)
{
	PROFILE();
	
	DATACHECK;
	
	IT_PATTERN *pat;
	unsigned char *p;
	IT_NOTE *n;
	IT_CHANNEL_LAST_INFO *last;
	unsigned char cv,ch,mask;

	if(pos==0) CLEARARRAY(ChLastInfo, it_channels);
	
	CLEARARRAY(rowinfo, it_channels);
	
	if(patnumber>=it_header.PatNum) return(0);

	pat=&patterns[patnumber];
	if(pat->Length==0){
		// ERROR
		return(0);
	}
	p=&pat->data[pos];


GetNextChannelMarker:
	cv=*p++; pos++; // Read byte into channelvariable
	if(cv==0) return pos; // return in current position
	ch=(cv-1)&63;
	
	n=&rowinfo[ch];
	last=&ChLastInfo[ch];
	if(cv&128)
		{
		last->lastmask=mask=*p++; pos++;
		}
		else 
		mask=last->lastmask;
		
	if(mask&1)
		{
		last->lastnote=n->note=*p++; pos++; // read note (0-119)
		n->isnote=0xFF;		
		}
	if(mask&2)
		{
		last->lastinstrument=n->instrument=*p++; pos++; // read instrument(1-99)
		}
	if(mask&4)
		{
		last->lastvolpan=n->voleffect=*p++;pos++; // read volume/panning
		n->isvoleffect=0xFF;
		}
	if(mask&8)
		{
		last->lastcommand=n->effect=*p++;pos++;//read command (0->31) "0-noeffect"
		last->lastcommanddata=n->effectdata=*p++;pos++;
		}
	if(mask&16)
		{
		n->note=last->lastnote;
		n->isnote=0xFF;
		}
	if(mask&32) n->instrument=last->lastinstrument;
	if(mask&64)
		{
		n->voleffect=last->lastvolpan;
		n->isvoleffect=0xFF;
		}
	if(mask&128)
		{
		n->effect=last->lastcommand;
		n->effectdata=last->lastcommanddata;
		}
	goto GetNextChannelMarker;

}

void ITModule::MoveToBackGround(TRACK *trk)
{
	PROFILE();	
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[trk->vchannel];

	if(trk->NoteVol<=0){
		KillChannel(trk->vchannel);
		return;
	}
	
	switch (trk->NNA){
		case NNA_OFF:
			DoNoteOff(trk->vchannel);
			break;
		case NNA_FAD:
			vc->fadeout=vc->inst->FadeOut;
			break;
		}
						
	
	vc->VirtualChannelVol=trk->ChannelVol;
	vc->VirtualNoteVol=trk->NoteVol;
	vc->VirtualNotePan=trk->NotePan;
	vc->isdirect=0;	
}

int ITModule::GetVirtualChannel(int host)
{
	PROFILE();
	DATACHECK;
	
	TRACK *trk=&track[host];
	int vch=-1;

	if(trk->active){
		if(trk->NNA==NNA_CUT){
			vch=trk->vchannel;
			goto GOTVIRTUALCHANNEL;
		}
		else{
			MoveToBackGround(trk);
		}	
	}

	vch = m_mixer->GetFreeChannel();
	if ((vch<0)||(vch>=m_numVirtualChannels)) {
		return -1;
	} else {
		m_mixer->ReserveSingleChannel(vch);
		m_mixer->SetMasterVolume(m_masterVolume,vch,1);
	}
	
GOTVIRTUALCHANNEL:
	VIRTUALCHANNEL *vc=&virtualchannels[vch];	
	memset(vc,0, sizeof(VIRTUALCHANNEL));	
	vc->isdirect=1;
	vc->host=host;
	
	return vch;
}	


/*
         1) The end of a sample is reached (quite obvious)
         2) When the end of a volume envelope is reached, and the final
            envelope volume is 0
         3) When the fadeout value for a channel causes it to become silent.
         4) When a duplicate note is played when DNT is set to Note and DCA is
            set to cut for the instrument.
         5) When a notecut is issued (obvious)
         6) When a channel is moved to the background (using NNAs)
            AND the volume is 0.
*/

void ITModule::KillChannel(int ch)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[ch];
	if(vc->active){
		vc->active=0;
		m_mixer->SetVoiceStatus(ch,0);
		m_mixer->FreeChannel(ch);
		if(vc->isdirect) track[vc->host].active=0;
	}
}

void ITModule::DoNoteOff(int channel)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[channel];
	if(!vc->keyon) return;
	vc->keyon=0;	

	if((InstrumentMode)&&(vc->active))
	{
		IT_INSTRUMENT *inst=vc->inst;
		IT_ENVELOPE *env = &inst->VolEnv;
		ENVCONTROL *ctrl = &vc->VolEnv;
		char ENV_ON=0,ENV_LOOP=0,ENV_SUSTAIN=0;
		if(env->Flg&1) ENV_ON=1;
		if((ENV_ON)&&(env->Flg&2)) ENV_LOOP=1;
		if((ENV_ON)&&(env->Flg&4)) ENV_SUSTAIN=1;
		if((ENV_ON)&&(env->Flg&2)) ENV_LOOP=1;

        PrepareEnvelopes(0, vc);

		if (ENV_ON==0 || ENV_LOOP)
			vc->fadeout=inst->FadeOut;
	}

	if(vc->active){
		if(!(vc->smp->Flg&SMP_IT_SUSTAINLOOP)) return;

		int loopmode=SOUND_LOOP_OFF;
		StaticSound *wv=vc->wv;
		IT_SAMPLE *smp=vc->smp;
		long begin=0,end=wv->GetNumFrames();

		if(smp->Flg&SMP_IT_LOOP){
			if(smp->Flg&SMP_IT_PINGPONGLOOP){
				loopmode=SOUND_LOOP_BIDI;
			} else {
				loopmode=SOUND_LOOP_NORMAL;
			}
			begin=smp->LoopBeg; end=smp->LoopEnd;
		}
		
		m_mixer->SetLoop(channel, begin, end, loopmode);
		
	}
				
}

/***************************************************************************
			PROCESS EFFECTS
 ***************************************************************************/
// test order: Dx0, D0x, DxF, DFx
void ITModule::DoVolumeSlide(int *Val,unsigned char dat, int upperlimit)
{
	PROFILE();
	DATACHECK;
	
	unsigned char nybble1,nybble2;
	nybble1=dat>>4;
	nybble2=dat&0x0F;

	if(nybble2==0) // test for Dx0 (volume slide up)
	{
		if(FirstTick){
			if(nybble1==0xF) *Val+=15;
		} else *Val+=nybble1;
	}else		
	if(nybble1==0) // test for D0x (volume slide down)
	{
		if(FirstTick){
			if(nybble2==0xF) *Val-=15;
		} else *Val-=nybble2;
	}
	else			
	if(nybble2==0xF) // test for DxF (fine volume slide up)
	{
		if(FirstTick) *Val+=nybble1;
	}
	else
	if(nybble1==0xF) // test for DFx (fine volume slide down)
	{
		if(FirstTick) *Val-=nybble2;
	}
		
	*Val=MIN(MAX(0,*Val),upperlimit);

}

void ITModule::DoPitchSlideDown(TRACK *trk,unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[trk->vchannel];
	unsigned char nybble1,nybble2;
	nybble1=dat>>4;
	nybble2=dat&0x0F;
	
	switch(nybble1){
		case 0xF: // test for EFx (fine pitch slide down)
			if(!FirstTick) break;
			vc->NoteFreq=fixed_X_int(vc->NoteFreq,
				              (unsigned int)m_core->IT_LinearSlideDownTable[nybble2]);
			break;
		case 0xE: // test for EEx (extra fine pitch slide down)
			if(!FirstTick) break;
			vc->NoteFreq=fixed_X_int(vc->NoteFreq,
			                      (unsigned int)m_core->IT_FineLinearSlideDownTable[nybble2]);
			break;
		default :
			if(FirstTick) break;
			vc->NoteFreq=fixed_X_int(vc->NoteFreq,
									(unsigned int)m_core->IT_LinearSlideDownTable[dat]);
			break;
		}
}


void ITModule::DoPitchSlideUp(TRACK *trk,unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[trk->vchannel];
	unsigned char nybble1,nybble2;
	nybble1=dat>>4;
	nybble2=dat&0x0F;
	
	switch(nybble1){
		case 0xF: // test for FFx (fine pitch slide up)
			if(!FirstTick) break;
			vc->NoteFreq=fixed_X_int(vc->NoteFreq,
				              (unsigned int)m_core->IT_LinearSlideUpTable[nybble2]);
			break;
		case 0xE: // test for FEx (extra fine pitch slide up)
			if(!FirstTick) break;
			vc->NoteFreq=fixed_X_int(vc->NoteFreq,
			                      (unsigned int)m_core->IT_FineLinearSlideUpTable[nybble2]);
			break;
		default:
			if(FirstTick) break;
			vc->NoteFreq=fixed_X_int(vc->NoteFreq,
			                      (unsigned int)m_core->IT_LinearSlideUpTable[dat]);
			break;
		}
}


void ITModule::DoVibrato(TRACK *trk, unsigned char dat,unsigned char finevibrato)
{
	PROFILE();
	DATACHECK;
	
//	VIRTUALCHANNEL *vc=&virtualchannel[trk->vchannel];
	unsigned char vibspd;
	int vibdepth,index,value,slide,fine;

	vibspd=dat>>4;
	vibdepth=dat&0x0F;
	trk->vibpos+=vibspd*4;
	if(!finevibrato)
		vibdepth*=4;
	
	if(OldEffects) vibdepth*=2;

	index=trk->vibpos&0xFF;	
	switch(trk->vibratowaveform){
		case 0:
			index=(int)m_core->IT_FineSineData[index];
			break;
		case 1:
			index=(int)m_core->IT_FineRampDownData[index];
			break;
		case 2:
			index=(int)m_core->IT_FineSquareWave[index];
			break;
		}

		
	value=abs(index)*vibdepth; // Val=index*depth/64
	
	value/=64;
	slide=(value>>2)&0xFF;
	fine=value&0x3;

// this one gets the frequency multipler 2^(Val/192)

	if(index>=0){		
		trk->VibFreqMul=fixed_X_int((unsigned int)m_core->IT_LinearSlideUpTable[slide],
				(unsigned int)m_core->IT_FineLinearSlideUpTable[fine]);
		}
		else{
		trk->VibFreqMul=fixed_X_int((unsigned int)m_core->IT_LinearSlideDownTable[slide],
				(unsigned int)m_core->IT_FineLinearSlideDownTable[fine]);
		}		
}


void ITModule::DoTonePortamento(TRACK *trk, unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[trk->vchannel];
	
	int freq=vc->NoteFreq, targetfreq=vc->TargetFreq;	
	
	if(FirstTick) return;
	if(freq==targetfreq) return;

	if(freq < targetfreq)
	{ // slide up
		vc->NoteFreq=fixed_X_int(freq,m_core->IT_LinearSlideUpTable[dat]);
		if(vc->NoteFreq>targetfreq) vc->NoteFreq=targetfreq;
	}
	else // slide down
	{
		vc->NoteFreq=fixed_X_int(freq,m_core->IT_LinearSlideDownTable[dat]);
		if(vc->NoteFreq<targetfreq) vc->NoteFreq=targetfreq;
	}
}		

void ITModule::DoTremor(TRACK *trk, unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
    unsigned char ontime=dat>>4,offtime=dat&0x0F;

    if(trk->tremorOFFTIME)
	{
        if(trk->tremorcount>=offtime)
        {
            trk->tremorOFFTIME=0;
            trk->tremorcount=0;
		}
	}
	else
	{
        if(trk->tremorcount>=ontime)
		{
            trk->tremorOFFTIME=1;
            trk->tremorcount=0;
		}
	}
		
	trk->tremorcount++;
}

void ITModule::DoArpeggio(TRACK *trk, unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
	int count;
	unsigned char a1=dat>>4,a2=dat&0x0F;
	if(FirstTick) trk->arpeggiocount=0;

	switch (trk->arpeggiocount)
		{
		case 0:	count=0;break;
		case 1:	count=a1;break;
		case 2: count=a2; break;
		default: count=0;
		}
		
		trk->ArpeggioMul=m_core->IT_PitchTable[12*5+count];

	trk->arpeggiocount++;
	if(trk->arpeggiocount>=3) trk->arpeggiocount=0;

}

void ITModule::DoRetrigg(TRACK *trk, unsigned char dat)
{
	PROFILE();	
	DATACHECK;
	
	unsigned char tickinterval=dat&0x0F;
	unsigned char volchange=dat>>4;
	int mch=trk->vchannel;

//	VIRTUALCHANNEL *vc=&virtualchannel[trk->vchannel];
	
	if(trk->retriggcount<=0)
		{
		trk->retriggcount=tickinterval;
		m_mixer->SetPosition(mch,0);

		switch (volchange){
			case 0:
			case 8:break;
			case 1: trk->NoteVol--;break;
			case 2: trk->NoteVol-=2;break;
			case 3: trk->NoteVol-=4;break;
			case 4: trk->NoteVol-=8;break;
			case 5: trk->NoteVol-=16;break;
			case 6: trk->NoteVol=(trk->NoteVol*2)/3;break; // *2/3
			case 7: trk->NoteVol/=2;break; // *1/2
			case 9: trk->NoteVol++;break;
			case 0xA: trk->NoteVol+=2;break;
			case 0xB: trk->NoteVol+=4;break;
			case 0xC: trk->NoteVol+=8;break;
			case 0xD: trk->NoteVol+=16;break;
			case 0xE: trk->NoteVol=(trk->NoteVol*3)/2;break;
			case 0xF: trk->NoteVol*=2;break;
			}

		trk->NoteVol=MIN(MAX(0,trk->NoteVol),64);
		}
	trk->retriggcount--;

}


void ITModule::DoTremolo(TRACK *trk,unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
	unsigned char spd,depth,index;
	int value=0;
	spd=dat>>4;
	depth=dat&0x0F;
	trk->trempos+=(spd<<2);

	index=(trk->trempos)&0xFF;	
	switch(trk->tremolowaveform){
		case 0:
			value=(int)m_core->IT_FineSineData[index];
			break;
		case 1:
			value=(int)m_core->IT_FineRampDownData[index];
			break;
		case 2:
			value=(int)m_core->IT_FineSquareWave[index];
			break;
		}

	value*=(int)depth;
	value/=32;
	trk->TremoloAdd=value;	
}

void ITModule::DoPanbrello(TRACK *trk,unsigned char dat)
{
	PROFILE();
	DATACHECK;
	
	unsigned char spd,depth,index;
	int value=0;
	spd=dat>>4;
	depth=dat&0x0F;
	trk->panbrellopos+=spd;

	index=trk->panbrellopos;
	switch(trk->panbrellowaveform){
		case 0:
			value=(int)m_core->IT_FineSineData[index];
			break;
		case 1:
			value=(int)m_core->IT_FineRampDownData[index];
			break;
		case 2:
			value=(int)m_core->IT_FineSquareWave[index];
			break;
		}

	value*=(int)depth;
	value/=32;
	trk->PanbrelloAdd=value;	
}



void ITModule::ProcessEffects(int channel)
{
	PROFILE();	
	DATACHECK;
	
	TRACK *trk=&track[channel];
//	VIRTUALCHANNEL *vc=&virtualchannel[trk->vchannel];
	unsigned char effect,dat,nybble1,nybble2,note;

	effect=trk->effect;
	dat=trk->effectdata;
	nybble1=dat>>4;
	nybble2=dat&0x0F;
	note=trk->note;
	
	uint8_t *ptr_mem;
	trk->isvibrato=0;
	trk->istremor=0;
	trk->isarpeggio=0;
	trk->istremolo=0;
	trk->ispanbrello=0;
		
	if(FirstTick){
		switch (note){
			case 254: // Notecut
//				CZLOG("NoteCut vchannel=%d\n", (int)trk->vchannel);
				if(trk->active)
					KillChannel(trk->vchannel);
				break;
			case 255: // noteoff
//				CZLOG("NoteOff vchannel=%d\n", (int)trk->vchannel);
				if(trk->active)
					DoNoteOff(trk->vchannel);
				break;
		}
	}
			

	if(FirstTick){
		if((trk->instpresent)&&(trk->active)){
			trk->NoteVol=trk->InitialNoteVol;
		}
	}
			
			
	DoVolEffects(trk);
	
	
	switch(effect){
		case 0: break;
		case 'A'- 64:	// Set Tempo
			if(!FirstTick) break;
			if(dat!=0) Speed=dat;
			TickCounter=Speed;
			break;

		case 'B'- 64:	// Jump to order
			if(!FirstTick) break;
			if(dat>=it_header.OrdNum) break;
			ProcessRow=NumberOfRows;
			ProcessOrder=dat;
			ProcessOrder--;
			break;

		case 'C'- 64:	// Break to row
			if(!FirstTick) break;
			ProcessRow=NumberOfRows;
			BreakRow=dat;			
			break;

		case 'D' - 64: // Volume Slide
			if (dat!=0) trk->volumeslide_mem=dat;
			if (trk->active)
				DoVolumeSlide(&trk->NoteVol,
					trk->volumeslide_mem,64);
			break;

		case 'E' - 64: // Pitch Slide down
			if(dat!=0) trk->pitchslide_mem=dat;
			if(trk->active)
				DoPitchSlideDown(trk,trk->pitchslide_mem);
			break;

		case 'F' - 64: // Pitch Slide up
			if(dat!=0) trk->pitchslide_mem=dat;
			if (trk->active)
				DoPitchSlideUp(trk,trk->pitchslide_mem);
			break;

		case 'G' - 64: // tone portamento to note
			ptr_mem=(GxxComp)? &trk->toneportamento_mem: &trk->pitchslide_mem;
			if(dat!=0) *ptr_mem=dat;
			if((trk->istoneportamento)&&(trk->active))
				DoTonePortamento(trk,*ptr_mem);
			break;

		case 'H' - 64: // vibrato
		case 'U' - 64: // fine vibrato
			trk->isvibrato=1;
			if(dat&0xF0) trk->vibrato_mem=(dat&0xF0)|(trk->vibrato_mem&0x0F);
			if(dat&0x0F) trk->vibrato_mem=(dat&0x0F)|(trk->vibrato_mem&0xF0);
			if((OldEffects)&&(FirstTick)) break;
			if (trk->active)
				DoVibrato(trk,trk->vibrato_mem,(effect=='H'-64)? 0:1);
			break;

		case 'I' - 64: // tremor with ontime x, offtime y
			trk->istremor=1;
			if(dat!=0) trk->tremor_mem=dat;
			if(trk->active) DoTremor(trk,trk->tremor_mem);
			break;

		case 'J' - 64: // arpeggio xy		
			trk->isarpeggio=1;
			if(dat!=0) trk->arpeggio_mem=dat;
			if(trk->active)
				DoArpeggio(trk,trk->arpeggio_mem);
			break;

		case 'K' - 64: // dual command Vibrato + Volumeslide
			trk->isvibrato=1;		
			if(dat!=0) trk->volumeslide_mem=dat;
			if(trk->active){
				DoVolumeSlide(&trk->NoteVol,trk->volumeslide_mem,64);
				if((OldEffects)&&(FirstTick)) break;
				DoVibrato(trk,trk->vibrato_mem,0);
				}
			break;

		case 'L' - 64: // dual command Portamento + Volumeslide
			ptr_mem=(GxxComp)? &trk->toneportamento_mem: &trk->pitchslide_mem;
			if(dat!=0) trk->volumeslide_mem=dat;
			if(trk->active){
				if(trk->istoneportamento) DoTonePortamento(trk,*ptr_mem);
				DoVolumeSlide(&trk->NoteVol,trk->volumeslide_mem,64);
				}
			break;

		case 'M' - 64: // set channel volume
			if(!FirstTick) break;
			if(dat<=0x40) trk->ChannelVol=dat;
			break;

		case 'N' - 64: // channel volume slide
			if(dat!=0) trk->channelvolumeslide_mem=dat;
			DoVolumeSlide(&trk->ChannelVol,trk->channelvolumeslide_mem,64);
			break;

		case 'O' - 64: // set (xx) sample offset  ( yxx00h )
			if(FirstTick)
				if(dat!=0){		
					unsigned int f=(unsigned int)dat<<8;
					unsigned int so=trk->sampleoffset&0xF0000;
					trk->sampleoffset=so|f;
					}
			break;
	
		case 'P' - 64:
			if(dat!=0) trk->panningslide_mem=(nybble2<<4)|nybble1;
			if(trk->active)
				DoVolumeSlide(&trk->NotePan,trk->panningslide_mem,64);
			break;
			
		case 'Q' - 64: // retrigg
			if(dat!=0) trk->retrigg_mem=dat;
			if(trk->active)
				DoRetrigg(trk,trk->retrigg_mem);
			break;

		case 'R' - 64: //tremolo Rxy(x=speed, y=depth)
			trk->istremolo=1;
			if(dat&0xF0) trk->tremolo_mem=(dat&0xF0)|(trk->tremolo_mem&0x0F);
			if(dat&0x0F) trk->tremolo_mem=(dat&0x0F)|(trk->tremolo_mem&0xF0);
			if(trk->active) DoTremolo(trk,trk->tremolo_mem);
			break;

		case 'S'- 64:
//			if(!firsttick) break;
			if(dat!=0) trk->Seffect_mem=dat;
			nybble1=trk->Seffect_mem>>4;
			nybble2=trk->Seffect_mem&0x0F;
			DoExtendedEffect(channel,trk,nybble1,nybble2);
			break;

		case 'T'- 64:
			if(dat!=0) trk->settempo_mem=dat;
			nybble1=trk->settempo_mem>>4;
			nybble2=trk->settempo_mem&0x0F;
			switch (nybble1){
				case 0: // slide down every non-row frame
					if(!FirstTick){
						
						//int tempo = czSoundDevice::GetTempo()-nybble2;
						int tempo = m_bpm-nybble2;
						
						tempo=MIN(MAX(32,tempo),255);
						
						//czSoundDevice::SetTempo(tempo);
						m_bpm = tempo;
						}
					break;
				case 1: // slide up every non-row frame
					if(!FirstTick){
						
						//int tempo=czSoundDevice::GetTempo()+nybble2;
						int tempo = m_bpm+nybble2;
						
						tempo=MIN(MAX(32,tempo),255);						
						
						//czSoundDevice::SetTempo(tempo);
						m_bpm = tempo;
						}
					break;
				default:
					//if(FirstTick) czSoundDevice::SetTempo(dat);
					if(FirstTick) m_bpm = dat;
					break;
				}
			break;
		
		case 'V' - 64:
			if(!FirstTick) break;
			if(dat<=0x80) GlobalVol=dat;
			break;

		case 'W'-64: // global volume slide
			if(dat!=0) trk->globalvolumeslide_mem=dat;
			DoVolumeSlide(&GlobalVol,trk->globalvolumeslide_mem,128);
			break;

		case 'X' - 64: // set panning
			if(FirstTick){
				int pan=((int)dat+1)/4;
				trk->ChannelPan=pan;
				if(trk->active)	trk->NotePan=pan;
				}
			break;

		case 'Y' - 64: // panbrello
			trk->ispanbrello=1;
			if(dat&0xF0) trk->panbrello_mem=(dat&0xF0)|(trk->panbrello_mem&0x0F);
			if(dat&0x0F) trk->panbrello_mem=(dat&0x0F)|(trk->panbrello_mem&0xF0);
			if(trk->active)	DoPanbrello(trk,trk->panbrello_mem);
			break;
		
		default:
			break;			
	}
  
}


void ITModule::DoVolEffects(TRACK *trk)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=NULL;
	unsigned char dat=0,temp;
	if (trk->active) vc=&virtualchannels[trk->vchannel];
	uint8_t *ptr_mem;
	
	int effect=trk->voleffect,eff=255;
	if(!trk->isvoleffect) return;
		
	if(inrange(effect,0,64)){eff=0;dat=effect-0;};
	if(inrange(effect,65,74)){eff=65;dat=effect-65;};
	if(inrange(effect,75,84)){eff=75;dat=effect-75;};
	if(inrange(effect,85,94)){eff=85;dat=effect-85;};
	if(inrange(effect,95,104)){eff=95;dat=effect-95;};
	if(inrange(effect,105,114)){eff=105;dat=effect-105;};
	if(inrange(effect,115,124)){eff=115;dat=effect-115;};
	if(inrange(effect,128,192)){eff=128;dat=effect-128;};
	if(inrange(effect,193,202)){eff=193;dat=effect-193;};
	if(inrange(effect,203,212)){eff=203;dat=effect-203;};

	
	switch(eff){
		case 0 : // set volume
			if(FirstTick)
				trk->NoteVol=dat;
			break;
		case 65: // fine volume slide up (DxF)
			if(dat!=0) trk->Vvolslide_mem=dat;
			temp=((trk->Vvolslide_mem<<4)&0xF0)|0xF;
			if(vc!=NULL) DoVolumeSlide(&trk->NoteVol,temp,64);
			break;
		case 75: // fine volume slide down (DFx)
			if(dat!=0) trk->Vvolslide_mem=0xF0+dat;
			temp=0xF0 + trk->Vvolslide_mem;
			if(vc!=NULL) DoVolumeSlide(&trk->NoteVol,temp,64);
			break;
		case 85: // volume slide up (Dx0)
			if(dat!=0) trk->Vvolslide_mem=dat;
			temp=(trk->Vvolslide_mem<<4)&0xF0;
			if(vc!=NULL) DoVolumeSlide(&trk->NoteVol,temp,64);
			break;
		case 95: // volume slide down (D0x)
			if(dat!=0) trk->Vvolslide_mem=dat;
			temp=trk->Vvolslide_mem&0x0F;
			if(vc!=NULL) DoVolumeSlide(&trk->NoteVol,temp,64);
			break;
		case 105: // pitch slide up (slide=x*4);
			if(dat!=0) trk->pitchslide_mem=dat*4;
			if(vc!=NULL) DoPitchSlideDown(trk,trk->pitchslide_mem);
			break;
		case 115: // pitch slide down (slide=x*4);
			if(dat!=0) trk->pitchslide_mem=dat*4;
			if(vc!=NULL) DoPitchSlideUp(trk,trk->pitchslide_mem);
			break;
		case 128: // pan
			if(FirstTick){
				trk->ChannelPan=dat;
				if (vc!=NULL) trk->NotePan=dat;
				}
			break;			
		case 193: // tone portamento
			ptr_mem=(GxxComp)? &trk->toneportamento_mem: &trk->pitchslide_mem;
			if(dat!=0) *ptr_mem=dat;
			if(vc!=NULL)
				if(trk->istoneportamento) DoTonePortamento(trk,*ptr_mem);
			break;
		case 203: // vibrato;
			trk->isvibrato=1;
			if(dat!=0) trk->vibrato_mem=(dat&0x0F)|(trk->vibrato_mem&0xF0);
			if(vc!=NULL) DoVibrato(trk,trk->vibrato_mem,0);
			break;
		
		}
}

void ITModule::DoExtendedEffect(int channel,TRACK *trk,unsigned char nybble1, unsigned char nybble2)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=NULL;
	switch(nybble1){
		case 0:
		case 1:
		case 2:
			break;
		case 3: trk->vibratowaveform=nybble2;break;
		case 4: trk->tremolowaveform=nybble2;break;
		case 5: trk->panbrellowaveform=nybble2;break;
		case 6:  // delay x ticks
			if(!FirstTick) break;		
			TickCounter+=nybble2;
			break;			
		case 7:
			if(!InstrumentMode) break;
			if(!FirstTick) break;
			if(trk->active) vc=&virtualchannels[trk->vchannel];
			int counter;
			switch (nybble2){
				case 0:
					vc=&virtualchannels[0];
					for(counter=0;counter<m_numVirtualChannels;counter++)
						{
						if(vc->active)
							if((vc->host==channel)&&(!vc->isdirect))
								KillChannel(counter);
						vc++;
						}
					break;
				case 1:
					vc=&virtualchannels[0];
					for(counter=0;counter<m_numVirtualChannels;counter++)
						{
						if(vc->active)
							if((vc->host==channel)&&(!vc->isdirect))
								if(vc->keyon)
									DoNoteOff(counter);
						vc++;
						}
					break;
				case 2:
					vc=&virtualchannels[0];
					for(counter=0;counter<m_numVirtualChannels;counter++)
						{
						if(vc->active)
							if((vc->host==channel)&&(!vc->isdirect))
								vc->fadeout=vc->inst->FadeOut;
						vc++;
						}
					break;
				case 3:	trk->NNA=NNA_CUT; break;
				case 4:	trk->NNA=NNA_CON; break;
				case 5:	trk->NNA=NNA_OFF; break;
				case 6:	trk->NNA=NNA_FAD; break;
				
				case 7:   if(vc!=NULL) vc->UseVolEnv=0;break;
				case 8:   if(vc!=NULL) vc->UseVolEnv=1;break;
				case 9:   if(vc!=NULL) vc->UsePanEnv=0;break;
				case 0xA: if(vc!=NULL) vc->UsePanEnv=1;break;
				case 0xB: if(vc!=NULL) vc->UsePitEnv=0;break;
				case 0xC: if(vc!=NULL) vc->UsePitEnv=1;break;
				}
			break;
		case 8:
			if(!FirstTick) break;
			trk->ChannelPan=m_core->IT_pantable[nybble2];
			if(trk->active)
				trk->NotePan=trk->ChannelPan;
			break;
		case 9: // set surround (not implemented)
			break;
		case 0xA: // set "y" part of setsampleoffset ( yxx00h )
			if(FirstTick){
				unsigned int tmp=(unsigned int)nybble2<<16;
				unsigned int mem=trk->sampleoffset&0xFFFF;
				trk->sampleoffset=tmp|mem;
				}
			break;
		case 0xB:
			if(!FirstTick) break;		
			switch (nybble2){
				case 0:
					trk->loopbackrow=ProcessRow;
					break;
				default:
					if(trk->loopbackcount<nybble2)
						{
						trk->loopback=1;
						trk->loopbackcount++;
						}
						else						
						trk->loopbackcount=0;
				}
			break;

		case 0xC: // note cut after x ticks
			if(FirstTick) trk->cutcountdown=nybble2;
			if(trk->cutcountdown)
				trk->cutcountdown--;
				else
				if(trk->active)	KillChannel(trk->vchannel);
			break;
		case 0xD: // note delay (handled by preparenote)
			break;
		case 0xE: // pattern delay for x rows
			if((FirstTick)&&(!PatternDelay)){
				RowCounter=1;
				RowCounter+=nybble2;
				}
			break;
		}

}




/*
The speed of the sample is how fast the vibrato is processed... the depth
is the maximum depth of vibrato.. the rate determines how fast the depth of
vibrato is applied. 

A rough CurrentVibratoDepth calculation would be:
 Accumulator += Rate;
 if(Accumulator / 256 > Depth) CurrentVibratoDepth = Depth;
 else CurrentVibratoDEpth = Accumulator / 256;
*/
unsigned int ITModule::DoSampleVibrato(VIRTUALCHANNEL *vc)
{
	PROFILE();	
	DATACHECK;
	
	unsigned char vibspd=vc->smp->ViS;  // 0-64
	unsigned char MaxDepth=vc->smp->ViD;     // 0-32
	int index,value,slide,fine;
	int CurrentDepth;
	
	vc->SampleVibratoAcum+=vc->smp->ViR;
	if(vc->SampleVibratoAcum/256 > MaxDepth) CurrentDepth=MaxDepth;
		else CurrentDepth=vc->SampleVibratoAcum/256;

	
	vc->samplevibpos+=vibspd;
	index=vc->samplevibpos&0xFF;	
	switch(vc->smp->ViT){
		case 0:
			index=(int)m_core->IT_FineSineData[index];
			break;
		case 1:
			index=(int)m_core->IT_FineRampDownData[index];
			break;
		case 2:
			index=(int)m_core->IT_FineSquareWave[index];
			break;
		}

		
	
	value=abs(index)*CurrentDepth; // Val=index*depth/64	
	value/=64;
	
	if(value==0) return 1L<<16L;
	
	slide=(value>>2)&0xFF;
	fine=value&0x3;

// this one gets the frequency multipler 2^(Val/192)

	if(index>=0){		
		return fixed_X_int((unsigned int)m_core->IT_LinearSlideUpTable[slide],
				(unsigned int)m_core->IT_FineLinearSlideUpTable[fine]);
		}
		else{
		return fixed_X_int((unsigned int)m_core->IT_LinearSlideDownTable[slide],
				(unsigned int)m_core->IT_FineLinearSlideDownTable[fine]);
		}		
}

void ITModule::PrepareOutput(int ch)
{
	PROFILE();	
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[ch];	
	if(!vc->active) return;

	IT_INSTRUMENT *inst=vc->inst;
	IT_SAMPLE *smp=vc->smp;	
	TRACK *trk=&track[vc->host];
	
	int ChannelVol= (vc->isdirect) ? trk->ChannelVol: vc->VirtualChannelVol;
	int NoteVol= (vc->isdirect) ? trk->NoteVol : vc->VirtualNoteVol;
	int NotePan= (vc->isdirect) ? trk->NotePan : vc->VirtualNotePan;


	vc->FinalFreq=vc->NoteFreq;	
	
	
	if(vc->isdirect){
		if(trk->isvibrato)
			vc->FinalFreq=fixed_X_int(vc->FinalFreq,trk->VibFreqMul);
		if(trk->isarpeggio)
			vc->FinalFreq=fixed_X_int(vc->FinalFreq,trk->ArpeggioMul);
		if((trk->istremor)&&(trk->tremorOFFTIME))
			NoteVol=0;
		if(trk->istremolo)
			{
			NoteVol+=trk->TremoloAdd;
			NoteVol=MIN(MAX(0,NoteVol),64);
			}
		if(trk->ispanbrello)
			{
			NotePan+=trk->PanbrelloAdd;
			NotePan=MIN(MAX(0,NotePan),64);
			}
		if(vc->smp->ViS){
			vc->FinalFreq=fixed_X_int(vc->FinalFreq,
					DoSampleVibrato(vc));
			}
		}


	//
	// UPDATE ENVELOPES
	//
	if(InstrumentMode)
	{
		if((vc->VolEnv.flag)&&(vc->UseVolEnv))
		{
			UpdateEnvelope(&inst->VolEnv,&vc->VolEnv);
			if(vc->VolEnv.flag==0)
			{
				if(FIXTOI16_16(vc->VolEnv.val_)<=0){
					KillChannel(ch);
					return;
				}					
				vc->fadeout=inst->FadeOut;
			}
		}

		if((vc->PanEnv.flag)&&(vc->UsePanEnv))
			UpdateEnvelope(&inst->PanEnv,&vc->PanEnv);
		if((vc->PitEnv.flag)&&(vc->UsePitEnv))
			UpdateEnvelope(&inst->PitEnv,&vc->PitEnv);			
		
		// Update Fadeout	
		vc->NFC -= vc->fadeout;
		if(vc->NFC<=0)
		{
			KillChannel(ch);
			return;
		}
	}

	//
	// CALCULATE FINAL VOLUME
	//
	if(InstrumentMode){
		/*
		uint64_t i1,i2;
		uint64_t d1=0;
		i1 = (uint64_t)NoteVol*(uint64_t)smp->GvL*
		    (uint64_t)inst->GbV*(uint64_t)ChannelVol;
		i2= (uint64_t)GlobalVol*(uint64_t)vc->NFC;
		d1= (i1*i2*(uint64_t)FIXTOI16_16(vc->VolEnv.val_)) >> 40;
		vc->FinalVol=MIN(255,(unsigned int)d1);
		*/
		TInt64 i1,i2;
		TInt64 d1=0;
		i1 = TInt64((int)NoteVol) * TInt64((int)smp->GvL) * TInt64((int)inst->GbV) * TInt64((int)ChannelVol);
		i2 = TInt64(GlobalVol) * TInt64(vc->NFC);
		d1= i1 * i2 * TInt64(FIXTOI16_16(vc->VolEnv.val_));
		TInt64_Lsr(d1,40);
		vc->FinalVol=MIN(255,(unsigned int)TInt64_GetTInt(d1));
	}
	else
	{	
		// impulse tracker uses >>18 (/262144), but we need >>17 (0-255 vol)
		vc->FinalVol=(NoteVol*(int)smp->GvL*ChannelVol*GlobalVol)>>17;
		vc->FinalVol=MIN(vc->FinalVol,255);
	}

	//
	// CALCULATE FINAL PAN
	//
	if (InstrumentMode){
		// pan = (pan-32) + Env*(32-abs(pan-32))
		
		fixed16_16 tmp1=FIX16_16(NotePan-32);
		fixed16_16 vabs= abs(tmp1);
		fixed16_16 v32= FIX16_16(32);
		
		fixed16_16 pan = tmp1 + FMUL16_16(vc->PanEnv.val_, FDIV16_16((v32-vabs),v32));
		pan +=v32;
		vc->FinalPan=MIN(255,FIXTOI16_16(pan)*4);
		
        				
	}else{
		vc->FinalPan=MIN(255,NotePan*4);
	}


	// CALCULATE FINAL FREQUENCY
	if(InstrumentMode){
		// Fixed point here is 18.14, because the vPow function
		/*
		uint8_t exp=FIXTOI16_16(vc->PitEnv.val_);

		// define as Mophun fixed32_t types, to use with Mophun's vPow function
		//#define HALFsemitone_forward 1.029302237
		fixed32_t HALFsemitone_forward  = (fixed32_t)((1.029302237f) * 16384);

		fixed32_t mul=vPow(HALFsemitone_forward, exp);
		//vc->FinalFreq = vFTOI((int64_t)vc->FinalFreq*(int64_t)mul);
		TInt64 v(TInt64(vc->FinalFreq) * TInt64(mul) );
		TInt64_Lsr(v,14);
		vc->FinalFreq = TInt64_GetTInt(v);
		*/
		
		#define HALFsemitone_forward 1.029302237f
		float exp = float(vc->PitEnv.val_) / (1<<16);
		float mul = pow(float(HALFsemitone_forward), exp );
		float ffreq = vc->FinalFreq*mul;
		vc->FinalFreq = (int)ffreq;
	}

//	vc->FinalPan=(int(it_header.Sep)*(pan-128))/128 +128;
	int pan=vc->FinalPan;	
	
	m_mixer->SetVolume(ch,vc->FinalVol);
	m_mixer->SetPanning(ch,(int(it_header.Sep)*(pan-128))/128 +128);	
	m_mixer->SetFrequency(ch,vc->FinalFreq);
	if(vc->turnON){
		m_mixer->SetVoiceStatus(ch,1);
		vc->turnON=0;
	}
	
}



/***************************************************************************
			ENVELOPE FUNCTIONS
 ***************************************************************************/
inline fixed16_16 Interpolate(int y1, int y2, int x1, int x2)
{
	//PROFILE();
		
//	CZLOG("Interpolate (%d-%d)/(%d-%d)\n", y2,y1,x2,x1);

	y1 = FIX16_16(y1);
	y2 = FIX16_16(y2);
	x1 = FIX16_16(x1);
	x2 = FIX16_16(x2);
	
	if(x1!=x2){
		return FDIV16_16((y2-y1), (x2-x1));
	}
	else{
//		CZLOG("Y1=%d Y2=%d X1=%d X2=%d\n", y1, y2, x1, x2);
		return FDIV16_16((y2-y1), FIX16_16(1));
//		return 0;		
	}
}

void ITModule::UpdateEnvelope(IT_ENVELOPE *env, ENVCONTROL *ctrl)
{
	PROFILE();
	DATACHECK;
	
	char ENV_ON=0,ENV_LOOP=0,ENV_SUSTAIN=0;
	if(env->Flg&1) ENV_ON=1;
	if((ENV_ON)&&(env->Flg&2)) ENV_LOOP=1;
	if((ENV_ON)&&(env->Flg&4)) ENV_SUSTAIN=1;

	if(!ENV_ON) return; // no envelope

	// If it the first process after a restart, then just exit
	if(ctrl->flag==-1){
		ctrl->flag=1;
		ctrl->ticksToNextNode--;
		return;
	}


	if (ctrl->ticksToNextNode<=0){ // Advance to next node
		int node1, node2;
		
		if (ctrl->currentNode < ctrl->endNode) // There is still nodes to process
		{	
			ctrl->currentNode++;
		}
		else
		{ // No more nodes to process
			// If there is no more nodes, and not in loop mode, then just disable envelope, and exit
			if(!ctrl->loop){
				// No loop, so the val stays equal to the last node
				//CZLOG("Reached end of envelope...\n");
				ctrl->flag=0;
				ctrl->val_ = FIX16_16(env->Nodes[ctrl->endNode].y);
				return;
			}						
			ctrl->currentNode = ctrl->repeatNode;
		}

		node1 = ctrl->currentNode;
		node2 = (ctrl->currentNode==ctrl->endNode) ? ctrl->endNode : ctrl->currentNode+1;
		
		ctrl->val_ = FIX16_16(env->Nodes[node1].y);
		ctrl->inc_=Interpolate(FIXXX16_16(env->Nodes[node1].y), FIXXX16_16(env->Nodes[node2].y),
			FIXXX16_16(env->Nodes[node1].tick), FIXXX16_16(env->Nodes[node2].tick));
		ctrl->ticksToNextNode = env->Nodes[node2].tick - env->Nodes[node1].tick;
			
	} else {
		// If we don't need to advance the node, just update some variables
		ctrl->val_+=ctrl->inc_;
		ctrl->ticksToNextNode--;
//		CZLOG("ctrl->val=%d\n", (int)ctrl->val_ >> 16);
	}
	
}

/*
 * keyon = 1 - sustain state
 * keyon = 0 - sustain off (note off command)
 */
void ITModule::PrepareEnvelope(uint8_t keyon,IT_ENVELOPE *env, ENVCONTROL *ctrl)
{
	PROFILE();
	DATACHECK;
	
	char ENV_ON=0,ENV_LOOP=0,ENV_SUSTAIN=0;

	if(env->Flg&1) ENV_ON=1;
	if((ENV_ON)&&(env->Flg&2)) ENV_LOOP=1;
	if((ENV_ON)&&(env->Flg&4)) ENV_SUSTAIN=1;

	if(!ENV_ON) return; // no envelope??

	// Is a envelope restart
	if(ctrl->flag==-1){	 
		// CZLOG("PrepareEnvelope Envelope Restart\n");
		ctrl->currentNode = 0;
		ctrl->repeatNode = 0;
		ctrl->endNode = env->Num-1;
		ctrl->ticksToNextNode = env->Nodes[ctrl->currentNode+1].tick - env->Nodes[ctrl->currentNode].tick;
		ctrl->loop = 0;
		ctrl->val_ = FIX16_16(env->Nodes[0].y);
		
	}
	
	// Update the loop nodes, but not alter the current position
	if ((keyon)&&(ENV_SUSTAIN)){
		ctrl->repeatNode = env->SLB;
		ctrl->endNode = env->SLE;
		ctrl->loop = 1;		
	} else if (ENV_LOOP){
		ctrl->repeatNode = env->LpB;
		ctrl->endNode = env->LpE;			
		ctrl->loop = 1;		
	} else {
		ctrl->repeatNode = 0;
		ctrl->endNode = env->Num-1;
		ctrl->loop=0;					
	}
	

	// Is a envelope restart, recalculate increment, with the new nodes
	if(ctrl->flag==-1){	 
		int node1,node2;
		node1 = ctrl->currentNode;
		node2 = (ctrl->currentNode==ctrl->endNode) ? ctrl->endNode : ctrl->currentNode+1;
		ctrl->inc_=Interpolate(FIXXX16_16(env->Nodes[node1].y), FIXXX16_16(env->Nodes[node2].y),
			FIXXX16_16(env->Nodes[node1].tick), FIXXX16_16(env->Nodes[node2].tick));
	}
	else if (ctrl->flag==1 && ctrl->ticksToNextNode==0)
	{
		int node1,node2;
		node1 = ctrl->currentNode;
		node2 = (ctrl->currentNode==ctrl->endNode) ? ctrl->endNode : ctrl->currentNode+1;
		ctrl->inc_=Interpolate(FIXXX16_16(env->Nodes[node1].y), FIXXX16_16(env->Nodes[node2].y),
			FIXXX16_16(env->Nodes[node1].tick), FIXXX16_16(env->Nodes[node2].tick));
		ctrl->ticksToNextNode = env->Nodes[node2].tick - env->Nodes[node1].tick;
	}
	
}

void ITModule::PrepareEnvelopes(uint8_t keyon, VIRTUALCHANNEL *vc)
{
    PrepareEnvelope(keyon, &vc->inst->VolEnv, &vc->VolEnv);
    PrepareEnvelope(keyon, &vc->inst->PanEnv, &vc->PanEnv);
    PrepareEnvelope(keyon, &vc->inst->PitEnv, &vc->PitEnv);              
}



void ITModule::DoDCT(int channel)
{
	PROFILE();	
	
TRACK *trk=&track[channel];
VIRTUALCHANNEL *vc=&virtualchannels[0];
int counter;
	switch(trk->DCT){
		case DCT_NTE:
			for(counter=0;counter<m_numVirtualChannels;counter++)
				{
				if((vc->active)&&(vc->host==channel)&&
				   (vc->insnum==trk->insnum)&&(vc->note==trk->note))
					{
					int DCA=trk->DCA;
					if(DCA==DCA_CUT){
						KillChannel(counter);
						goto DONEDCT;
						}
					if((DCA==DCA_OFF)&&(vc->keyon)){
						DoNoteOff(counter);
						goto DONEDCT;
						}
					if((DCA==DCA_FAD)&&(!vc->fadeout)){
						vc->fadeout=vc->inst->FadeOut;
						goto DONEDCT;
						}
					}
				vc++;
				}
			break;
		case DCT_SMP:
			for(counter=0;counter<m_numVirtualChannels;counter++)
				{
				if((vc->active)&&(vc->host==channel)&&
				   (vc->insnum==trk->insnum)&&(vc->smpnum==trk->smpnum))
					{
					int DCA=trk->DCA;
					if(DCA==DCA_CUT){
						KillChannel(counter);
						goto DONEDCT;
						}
					if((DCA==DCA_OFF)&&(vc->keyon)){
						DoNoteOff(counter);
						goto DONEDCT;
						}
					if((DCA==DCA_FAD)&&(!vc->fadeout)){
						vc->fadeout=vc->inst->FadeOut;
						goto DONEDCT;
						}
					}
				vc++;
				}
			break;
		case DCT_INS:
			for(counter=0;counter<m_numVirtualChannels;counter++)
				{
				if((vc->active)&&(vc->host==channel)&&
				   (vc->insnum==trk->insnum))
					{
					int DCA=trk->DCA;
					if(DCA==DCA_CUT){
						KillChannel(counter);
						goto DONEDCT;
						}
					if((DCA==DCA_OFF)&&(vc->keyon)){
						DoNoteOff(counter);
						goto DONEDCT;
						}
					if((DCA==DCA_FAD)&&(!vc->fadeout)){
						vc->fadeout=vc->inst->FadeOut;
						goto DONEDCT;
						}
					}
				vc++;
				}
			break;

	}
	
	DONEDCT:
	return;	
}

void ITModule::KickNote(int channel)
{
    PROFILE();

    TRACK *trk=&track[channel];
    IT_INSTRUMENT *inst=&instruments[trk->insnum-1];
    IT_SAMPLE *smp=&samples[trk->smpnum-1];
    StaticSound *wv=waves[trk->smpnum-1];

    trk->kick=0;

    int newfreq=fixed_X_int(smp->C5Speed,m_core->IT_PitchTable[trk->realnote]);

    trk->istoneportamento=0;
    if((trk->active)&&((trk->effect=='G'-64)||(trk->effect=='L'-64)))
        trk->istoneportamento=1;
    if((trk->active)&&inrange(trk->voleffect,193,202))
        trk->istoneportamento=1;

    if (trk->istoneportamento)
    {
        VIRTUALCHANNEL *vc=&virtualchannels[trk->vchannel];
        vc->TargetFreq=newfreq;
        if((GxxComp)&&(trk->instpresent)&&(InstrumentMode)){
            vc->keyon=1;
            vc->VolEnv.flag=-1;
            vc->PanEnv.flag=-1;
            vc->PitEnv.flag=-1;
            vc->NFC=1024;
            vc->fadeout=0;
            PrepareEnvelopes(1, vc);
        }
        if((GxxComp)&&(trk->instpresent)){
            // Calculate initial PANNING
            int notepan=trk->ChannelPan;
			if(InstrumentMode && !(inst->DfP&128))
				notepan=inst->DfP&127;
            if(smp->DfP&128)
				notepan=(smp->DfP&127);
            if(InstrumentMode){
                int note=trk->note,PPC=inst->PPC,PPS=inst->PPS;
                notepan+=(note-PPC)*PPS/8;
            }
			trk->NotePan= CLAMP(notepan, 0, 64);
        }
        if(trk->instpresent){		
            trk->NoteVol=trk->InitialNoteVol;
        }

		//return;
    }

	int vch = trk->vchannel;
	if (trk->istoneportamento==0)
	{
	    vch= trk->vchannel = GetVirtualChannel(channel);	
	    if(vch==-1) {
			// there's inst an available channel
			trk->active=0;
			return;
		}
	}

    VIRTUALCHANNEL *vc=&virtualchannels[trk->vchannel];

    vc->inst=inst;
    vc->smp=smp;
    vc->wv=wv;


    if(trk->instpresent)
	{
		trk->NoteVol=trk->InitialNoteVol;
	}
    
	if (trk->istoneportamento==0)
	{
		vc->NoteFreq=newfreq;
		// Calculate initial PANNING
		int notepan=trk->ChannelPan;
		if(InstrumentMode && !(inst->DfP&128))
			notepan=inst->DfP&127;
		if(smp->DfP&128)
			notepan=(smp->DfP&127);
		if(InstrumentMode){
			int note=trk->note,PPC=inst->PPC,PPS=inst->PPS;
			notepan+=(note-PPC)*PPS/8;
		}
		trk->NotePan= CLAMP(notepan, 0, 64);

	}

    int loopmode=SOUND_LOOP_OFF;
    long repeat=0,end=wv->GetNumFrames();

    if(smp->Flg&SMP_IT_SUSTAINLOOP)
    {
        if(smp->Flg&SMP_IT_PINGPONGSUSTAINLOOP){
            loopmode=SOUND_LOOP_BIDI;
        } else {
            loopmode=SOUND_LOOP_NORMAL;
        }
        repeat=smp->SusLBeg; end=smp->SusLEnd;
    }
    else if(smp->Flg&SMP_IT_LOOP)
    {
        if(smp->Flg&SMP_IT_PINGPONGLOOP){
            loopmode=SOUND_LOOP_BIDI;
        } else {
            loopmode=SOUND_LOOP_NORMAL;
        }
        repeat=smp->LoopBeg; end=smp->LoopEnd;		
    }

	if (trk->istoneportamento==0)
	{
		if(InstrumentMode){
			trk->NNA=inst->NNA;
			trk->DCT=inst->DCT;
			trk->DCA=inst->DCA;
			DoDCT(channel);
		}

		vc->keyon=1;
		if(InstrumentMode){
			vc->NFC=1024;
			vc->fadeout=0;
			vc->VolEnv.flag=-1;
			vc->VolEnv.val_ = FIX16_16(64);
			vc->PitEnv.flag=-1;
			vc->PitEnv.val_ = FIX16_16(0);
			vc->PanEnv.flag=-1;
			vc->PanEnv.val_ = FIX16_16(0);
			vc->UseVolEnv=1;
			vc->UsePanEnv=1;
			vc->UsePitEnv=1;
			PrepareEnvelopes(1,vc);
		}

		vc->note=trk->note;
	}

    vc->insnum=trk->insnum;
    vc->smpnum=trk->smpnum;
    trk->VibFreqMul= FIX16_16(1);
	trk->active=1;
    vc->active=1;
    vc->turnON=1;	
    vc->SampleVibratoAcum=0;
    vc->samplevibpos=0;

    long begin=0;
    if(trk->effect=='O'-64)
    {
        if(trk->effectdata!=0){		
            unsigned int f=(unsigned int)trk->effectdata<<8;
            unsigned int so=trk->sampleoffset&0xF0000;
            trk->sampleoffset=so|f;
        }
        if(trk->sampleoffset<(unsigned int)end){
            begin=trk->sampleoffset;
        }else{
            if(OldEffects) begin=end;
        }
    }

    m_mixer->SetSample(vch,wv,begin,end,repeat,loopmode);   
}


void ITModule::PrepareNote(int channel)
{
	PROFILE();	
	DATACHECK;
	
	IT_NOTE *n=&rowinfo[channel];
	TRACK *trk=&track[channel];
	uint8_t kick=0;
	
	if(n->isnote)
		{
		trk->note=n->note;
		if(n->note<=119) kick=1;
		}

	if(n->instrument)
		{
		int i=n->instrument;
		int s=0;
		int max=(InstrumentMode) ? it_header.InsNum: it_header.SmpNum;
		if (i>max) i=0;
		if(i){
			if(InstrumentMode)
				s=instruments[i-1].KybdTbl[trk->note].sample;
			else
				s=i;
			}		
		
		if(s){
			trk->InitialNoteVol=samples[s-1].Vol;			
			trk->instpresent=1;
			if ((trk->note)&&(s!=trk->smpnum)) kick=1;
			}

		trk->insnum=i;
		trk->smpnum=s;
		}
	else trk->instpresent=0;
		
	trk->voleffect=n->voleffect;
	trk->isvoleffect=n->isvoleffect;
	trk->effect=n->effect;
	trk->effectdata=n->effectdata;

	
	if(n->isvoleffect)
		if(n->voleffect<=64)
			trk->InitialNoteVol=n->voleffect;
	

	if(kick){
		if((trk->insnum==0)||(trk->smpnum==0)){
			trk->instpresent=0;
			kick=0;
			}
		else
		if(!(samples[trk->smpnum-1].Flg&SMP_IT_SAMPLE))
			{
			trk->smpnum=0;
			trk->instpresent=0;
			kick=0;
			}
		}
			
	if (kick){
		trk->realnote=trk->note;
		if (InstrumentMode) trk->realnote=instruments[trk->insnum-1].KybdTbl[trk->note].note;
		}
	
// CHECK FOR DELAY
	if(n->effect=='S'-64)
		{
		if(n->effectdata!=0) trk->Seffect_mem=n->effectdata;
		if(trk->Seffect_mem>>4==0xD) // note delay;
			trk->notedelay=trk->Seffect_mem&0xF;
		}
		else
		trk->notedelay=0;

	trk->kick=kick;
		
}


void ITModule::CheckChannels(void)
{
	PROFILE();
	DATACHECK;
	
	VIRTUALCHANNEL *vc=&virtualchannels[0];
	
	for (int vch=0;vch<m_numVirtualChannels;vch++)
	{	
		if (vc->active)
		{
			if (!m_mixer->IsVoiceON(vch))
			{
				vc->active=0;
				m_mixer->FreeChannel(vch);
				if(vc->isdirect) track[vc->host].active=0;
			}
		}
		vc++;
	}
}
/***************************************************************************
  -------------------------------------------------------------------------
             		    IT PLAYER
  -------------------------------------------------------------------------
 ***************************************************************************/


void ITModule::DoTick(void)
{
	PROFILE();
	DATACHECK;
	
	CheckChannels();
	FirstTick=0;
	TickCounter--;
	if(TickCounter==0){
		TickCounter=Speed;
		FirstTick=1;
		RowCounter--;
		if(RowCounter!=0) goto UPDATE_EFFECTS; // if there are rows to repeat
		PatternDelay=0;		
		RowCounter=1;

        // do loopback if necessary
		uint8_t loopback=0,loopbackrow=0;
		int ch;
		for (ch=0;ch<it_channels;ch++){
			if (track[ch].loopback){
                loopback=1;
                loopbackrow=track[ch].loopbackrow;
                break;
            }
        }
        
        
		if(loopback){
			track[ch].loopback=0;
			int lines=ProcessRow=loopbackrow;
			NextPackPos=0;			
			while (lines){
				NextPackPos=GetITRow(CurrentPattern,NextPackPos);
				lines--;
            }
        } else{
            ProcessRow++;
        }
		
		if(ProcessRow>=NumberOfRows){
			ProcessRow=BreakRow;
			BreakRow=0;
			ProcessOrder++;
			PatternEnd=1;
			while (Orders[ProcessOrder]==0xFE) ProcessOrder++;
			if ((Orders[ProcessOrder]==0xFF)||((ProcessOrder-1)==m_lastOrder))
			{
				if(m_loop){
                    ProcessOrder=m_firstOrder;
                }else{
					m_reachedEnd = true;
					Stop();
					return;
				}
			}
			CurrentPattern=Orders[ProcessOrder];

			NextPackPos=0;
			int lines=ProcessRow;
			while (lines--) NextPackPos=GetITRow(CurrentPattern,NextPackPos);
			for(int ch=0;ch<it_channels;ch++) track[ch].loopbackrow=0;
			}
		CurrentRow=ProcessRow;
		NextPackPos=GetITRow(CurrentPattern,NextPackPos);
		NumberOfRows=patterns[CurrentPattern].Rows;
	}

UPDATE_EFFECTS:
	int ch;
	if ((FirstTick)&&(!PatternDelay))
		for (ch=0;ch<it_channels;ch++) PrepareNote(ch);


	for (ch=0;ch<it_channels;ch++)
		if ((track[ch].kick)&&(!track[ch].notedelay)) KickNote(ch);

	for (ch=0;ch<it_channels;ch++)
		if(!track[ch].notedelay) ProcessEffects(ch);
	
	if(RowCounter>1) PatternDelay=1;	

	for (ch=0;ch<m_numVirtualChannels;ch++)
		if (virtualchannels[ch].active)
			PrepareOutput(ch);

	for (ch=0;ch<it_channels;ch++)
		if(track[ch].notedelay) track[ch].notedelay--;
	
}






//--------
//
//  EXTRA FUNCTIONS
//
//--------
#if CZ_PLAYER_EXTRAFUNCTIONS_ENABLED

/*
int ITModule::GetFirstMixerChannel(void){
	return m_firstMixerChannel;
}
*/

char *ITModule::GetModuleName(void)
{
	PROFILE();		
	return it_header.modname;
}

int ITModule::GetChannels(void)
{
	PROFILE();
	return it_channels;
}

int ITModule::GetPosition(void)
{
	PROFILE();	
	return ProcessOrder;
}

int ITModule::GetOrders(void)
{
	PROFILE();
	return it_header.OrdNum-1;
}

int ITModule::GetPattern(void)
{
	PROFILE();
	return CurrentPattern;
}

int ITModule::GetPatternRows(void)
{
	PROFILE();		
	return NumberOfRows;
}

int ITModule::GetRow(void)
{
	PROFILE();		
	return CurrentRow;
}

int ITModule::GetGlobalVolume(void)
{
	PROFILE();
	return GlobalVol;
}


int ITModule::GetSpeed(void)
{
	PROFILE();
	return Speed;
}


int ITModule::SetMasterVolume(uint8_t vol){
	m_masterVolume = vol;
	for (int i=0;i<m_numVirtualChannels;i++){
		if (virtualchannels[i].active) m_mixer->SetMasterVolume(vol, i, 1);
	}
	return ERR_OK;
}

/*
int ITModule::Forward(void)
{
	PROFILE();	
	
	if(!IsPlaying) CZERROR(ERR_CANTRUN);
	
	uint8_t dopause=IsPaused;
	if (dopause) Pause();
	uint8_t mvol=m_mixer->GetMasterVolume(0);
	m_mixer->SetMasterVolume(0,0,SoundCardChannelsUsed);
	while(1){
		ENTER_CRITICAL;
		this->DoTick();
		LEAVE_CRITICAL;
		if((PatternEnd)||(m_reachedEnd))
		{
			PatternEnd=0;
			break;
		}
	}	
	m_mixer->SetMasterVolume(mvol,0,SoundCardChannelsUsed);

	if (dopause) Pause();
	return ERR_OK;
}

int ITModule::Backward(void)
{
	PROFILE();
	
	if(!IsPlaying) CZERROR(ERR_CANTRUN);

	uint8_t dopause=IsPaused;
	if (dopause) Pause();

	ENTER_CRITICAL;
	if((ProcessRow<=NumberOfRows/4)&&(ProcessOrder>0)) ProcessOrder--;
	for (int count=0;count<SoundCardChannelsUsed;count++){
		m_mixer->SetVoiceStatus(count,0);
	}

	ProcessRow=-1;
	TickCounter=1;
	RowCounter=1;
	CurrentPattern=Orders[ProcessOrder];
	NumberOfRows=patterns[CurrentPattern].Rows;
	BreakRow=0;
	CurrentRow=0;
	FirstTick=0;
	NextPackPos=0;
	PatternDelay=0;
	PatternEnd=0;

	this->DoTick();
	LEAVE_CRITICAL;

	if (dopause) Pause();
	
	return ERR_OK;
}
*/

#endif // CZ_PLAYER_EXTRAFUNCTIONS_ENABLED



} // namespace microaudio
} // namespace cz



#endif // CZ_PLAYER_IT_ENABLED
