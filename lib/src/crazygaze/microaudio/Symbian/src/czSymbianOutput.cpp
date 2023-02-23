
#include "czSymbianOutput.h"
#include "../czStandardC.h"

#ifdef new
	#undef new
#endif

_LIT(KMixer, "czMixer");

namespace cz
{
namespace audio
{

#if defined(CZPLAYER_SYMBIAN_EMULATOR) || defined(UIQ_BUILD)
	#define CZ_DELAY_FIX
#endif

//---------------------------------------------------------
// MyActive class is only used inside this file, so no need to have a seperate h/cpp
//---------------------------------------------------------
class MyActive : public CActive
{
public:
	void Request();
	MyActive(void);
	private:
	void DoCancel();
	void RunL();
};
MyActive::MyActive(void): CActive(CActive::EPriorityHigh)
{
	// adds the active to the queue
	CActiveScheduler::Add(this) ; 
}
// Cancels this task
void MyActive::DoCancel()
{
	TRequestStatus* status = &iStatus;
	RThread().RequestComplete(status,KErrCancel);
}
// Request accepting function
void MyActive::Request()
{
	SetActive();  
	iStatus = KRequestPending;
}
// Informs the requestor that there is a user activity
// or timer is fired
void MyActive::RunL()
{
	if(iStatus == KErrNone)
	{
		CActiveScheduler::Stop();
		Request();
	}
}
//
//
//
//

#ifdef CZ_DELAY_FIX

TInt64 Symbian_GetMicroseconds()
{
	TTime timeNow;
	timeNow.HomeTime();
	TDateTime midnight(timeNow.DateTime().Year(),timeNow.DateTime().Month(), timeNow.DateTime().Day(),0,0,0,0);
	TTimeIntervalMicroSeconds microseconds = timeNow.MicroSecondsFrom(TTime(midnight));
	return microseconds.Int64();
}

#endif


//	---------------------------------------------------------
// soundPlayer - Class that handles the symbian audio calbacks
//	---------------------------------------------------------
class soundPlayer : public CBase, public MMdaAudioOutputStreamCallback
{
public:
	soundPlayer();
    TRequestStatus *iStatus;
	TSharedData	*m_shared;
    TInt8 *iSoundBuf;
    TPtr8 iAudioData;
    TMdaAudioDataSettings	iAudioSettings;
    CMdaAudioOutputStream	*iAudioStream;

#ifdef CZ_DELAY_FIX
	TInt64 m_queuedTimeMicroseconds;
	TInt64 m_timeAtFillBufferMicroseconds;
#endif
		
	virtual void MaoscOpenComplete(TInt aError);
	virtual void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);
	virtual void MaoscPlayComplete(TInt aError);   
	void FillBuffer();
};

soundPlayer::soundPlayer() : iAudioData(NULL,0)
{

#ifdef CZ_DELAY_FIX
	m_queuedTimeMicroseconds = 0;
	m_timeAtFillBufferMicroseconds = 0;
#endif

}

void soundPlayer::FillBuffer()
{
	// wait for access to shared data
	m_shared->iMutex.Wait();  
	if (m_shared->m_fillWithSilence)
		iAudioData.FillZ();
	else
		m_shared->iDriver->FillBuffer(iSoundBuf, m_shared->settings.bufSizeFrames);
	// give access to shared data
	m_shared->iMutex.Signal();

#ifdef CZ_DELAY_FIX
	m_queuedTimeMicroseconds += m_shared->settings.bufSizeMicroseconds;
	m_timeAtFillBufferMicroseconds = Symbian_GetMicroseconds();
#endif

}

void soundPlayer::MaoscOpenComplete( TInt aError )
{
	m_shared->m_threadStarted = true;
	
	if( aError != KErrNone )
	{
		CActiveScheduler::Stop();
		return;
	}
	iAudioStream->SetAudioPropertiesL(m_shared->settings.sampleRate, m_shared->settings.numChannels );
#ifdef UIQ_BUILD
	iAudioStream->SetVolume( iAudioStream->MaxVolume() );
#else
	iAudioStream->SetVolume( iAudioStream->MaxVolume() >> 2);
#endif
	iAudioStream->WriteL(iAudioData);
}

//	---------------------------------------------------------
void soundPlayer::MaoscBufferCopied( TInt aError, const TDesC8 &aBuffer )
{
	if(!m_shared->m_playSound)
		CActiveScheduler::Stop();

	if( aError != KErrNone )
		return;

	if(m_shared->m_playSound )
	{

		// Keep the old time, because we need it after the call to FillBuffer
#ifdef CZ_DELAY_FIX
		TInt64 oldTimeAtFillBuffer = m_timeAtFillBufferMicroseconds;
#endif

		FillBuffer();

		// NOTE : This needs to be after FillBuffer, because if it depends on "m_timeAtFillBufferMicroseconds", which
		//        is updated inside FillBuffer
#ifdef CZ_DELAY_FIX
		m_queuedTimeMicroseconds -= (m_timeAtFillBufferMicroseconds - oldTimeAtFillBuffer);
		if (m_queuedTimeMicroseconds<0)
			m_queuedTimeMicroseconds = 0;
#endif

		iAudioStream->WriteL(iAudioData);

#ifdef CZ_DELAY_FIX
		if (m_queuedTimeMicroseconds > (m_shared->settings.bufSizeMicroseconds * 3)/ 2)
			User::After(m_shared->settings.bufSizeMicroseconds);
#endif

	}
}

//	---------------------------------------------------------
void soundPlayer::MaoscPlayComplete( TInt aError )
{
#ifdef CZ_DELAY_FIX
	m_queuedTimeMicroseconds = 0;
#endif

	if(m_shared->m_playSound && aError == KErrUnderflow )
	{
		iAudioStream->Stop();		
		iAudioStream->SetAudioPropertiesL(TMdaAudioDataSettings::ESampleRate8000Hz,TMdaAudioDataSettings::EChannelsMono);			
#ifdef UIQ_BUILD
	iAudioStream->SetVolume( iAudioStream->MaxVolume() );
#else
	iAudioStream->SetVolume( iAudioStream->MaxVolume() >> 2);
#endif
		FillBuffer();
		iAudioStream->WriteL(iAudioData);
	}
	else if(aError != KErrNone)
	{
		m_shared->m_working = false;
	}
	else
	{
		CActiveScheduler::Stop();
	}
}



//	---------------------------------------------------------
//	---------------------------------------------------------
void PlayL(TSharedData *m_shared)
{
	//	Install an active scheduler
	CActiveScheduler	*as = new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(as);
		
	soundPlayer *player = new(ELeave) soundPlayer;

	player->m_shared = m_shared;

	// Setup the buffer to where mixing is done, and make a first fill
	int bufferSizeBytes = player->m_shared->settings.bufSizeFrames*2;
	player->iSoundBuf = new( ELeave )TInt8[bufferSizeBytes];
	memset(player->iSoundBuf, 0, bufferSizeBytes);
	player->iAudioData.Set((TUint8*)player->iSoundBuf, bufferSizeBytes, bufferSizeBytes);
	
	memset( &player->iAudioSettings, 0, sizeof( TMdaAudioDataSettings));
	
	player->iAudioSettings.Query();
	player->iAudioStream = CMdaAudioOutputStream::NewL(*player);

	m_shared->m_threadRunning = true;

	MyActive* theActive = new (ELeave) MyActive;
	theActive->Request();
	m_shared->iStatusPtr = &(theActive->iStatus);

	// Fill the buffer for the first time
	player->FillBuffer();
	
	if( player->iAudioStream != NULL )
		player->iAudioStream->Open( &player->iAudioSettings );
	else
		m_shared->m_threadStarted = true;

	CActiveScheduler::Start();

	if(player)
	{	
		if(player->iAudioStream)
			delete player->iAudioStream;
		delete[] player->iSoundBuf;
		delete player;
	}

	if(theActive)
	{
		theActive->Cancel();  
		delete theActive;
	}
	delete as;

	m_shared->iAliveMutex.Signal();
}

TInt ThreadFunction( TAny *aPtr )
{
	TSharedData	*td = (TSharedData*)aPtr;

	td->m_threadRunning  = true;	
	td->iAliveMutex.Wait();

	CTrapCleanup	*cleanup = CTrapCleanup::New();
	if( cleanup != NULL )
	{
		TRAPD( error, PlayL( td ) );
	} else
	{
		td->m_threadStarted = true;
	}

	delete cleanup;
	td->m_threadRunning = false;
	return 0;
}



//
//
//
//
//
//
	
SymbianOutput::SymbianOutput(cz::Core *parentObject)  : SoundOutput(parentObject)
{
	PROFILE();
	m_shared.m_working = true;
	m_shared.m_threadStarted = false;
	m_shared.m_threadRunning = false;	
	m_open = false;
	m_paused = false;
}

SymbianOutput::~SymbianOutput()
{	
	PROFILE();

    if (m_open)
    {
    	StopAudio();
    }
    
    /*
	SendCmd( ECmdDestroyMixer );
	iShared.iAliveMutex.Wait();
	iShared.iAliveMutex.Close();
	iShared.iMutex.Close();	
	*/
}

bool ConvertFreqToEnum(int freq, TMdaAudioDataSettings::TAudioCaps *destEnum)
{
	// sample rate
	switch (freq)
	{
		case 8000:
			*destEnum = TMdaAudioDataSettings::ESampleRate8000Hz;
		break;
		case 11025:
			*destEnum = TMdaAudioDataSettings::ESampleRate11025Hz;
		break;
		case 16000:
			*destEnum = TMdaAudioDataSettings::ESampleRate16000Hz;
		break;
		case 22050:
			*destEnum = TMdaAudioDataSettings::ESampleRate22050Hz;
		break;
		case 32000:
			*destEnum = TMdaAudioDataSettings::ESampleRate32000Hz;
		break;
		case 44100:
			*destEnum = TMdaAudioDataSettings::ESampleRate44100Hz;
		break;
		case 48000:
			*destEnum = TMdaAudioDataSettings::ESampleRate48000Hz;
		break;
		default:
			return false;
	}
	
	return true;
}



int SymbianOutput::Init(int maxActiveSounds, int mixSizeMs, bool stereo,  bool bits16, int freq )
{
	if (!bits16) CZERROR(ERR_INVPAR);
	
	if (m_shared.m_threadRunning || m_open) CZERROR(ERR_CANTRUN);
		

	// Validate the frequency
	if (!ConvertFreqToEnum(freq, &m_shared.settings.sampleRate))
		CZERROR(ERR_INVPAR);
	m_shared.settings.numChannels = (stereo) ? TMdaAudioDataSettings::EChannelsStereo : TMdaAudioDataSettings::EChannelsMono;
	m_shared.settings.bufSizeFrames = (freq*mixSizeMs)/1000;
	m_shared.settings.bufSizeMicroseconds = mixSizeMs*1000;
	m_shared.iDriver = this;
	m_shared.iStatusPtr = (TRequestStatus *) 0x12345678;
	m_shared.m_playSound = true;

	// Init the mixer stuff in the parent class
	int ret = InitSoftwareMixerOutput(maxActiveSounds, m_shared.settings.bufSizeFrames, stereo, bits16, freq);
	if (ret!=ERR_OK) CZERROR(ret);
	
	StartAudio();
	
	return ERR_OK;
}

int SymbianOutput::StartAudio()
{
	if(!m_shared.m_threadRunning )
	{
		m_shared.m_fillWithSilence = false;
		m_shared.m_playSound = true;
		User::LeaveIfError( m_shared.iAliveMutex.CreateLocal( 1 ) );
		User::LeaveIfError( m_shared.iMutex.CreateLocal() );
		User::LeaveIfError(m_thread.Create(_L("czSoundThread"), ThreadFunction, KDefaultStackSize,KMinHeapSize,KMinHeapSize+1000000, &m_shared));

		m_thread.SetPriority(EPriorityRealTime);
		m_shared.m_threadStarted = false;
		m_thread.Resume();

		m_shared.m_threadStarted = false;
		m_open = true;
		return ERR_OK;
	}
	else
	{
		CZERROR(ERR_CANTRUN);
	}
}

int SymbianOutput::StopAudio()
{
	if(m_shared.m_threadRunning )
	{
		//	--- Stop playing sound
		m_shared.m_playSound = false;

		TRequestStatus* status = m_shared.iStatusPtr;
		if(status->Int() == KRequestPending )
			m_thread.RequestComplete(status, KErrNone);

		m_shared.m_threadRunning = false;
		m_thread.Close();

		m_shared.iAliveMutex.Wait();
		m_shared.iAliveMutex.Close();
		m_shared.iMutex.Close();
	    return ERR_OK;
	}
	else
	{
		CZERROR(ERR_CANTRUN);
	}
}


void SymbianOutput::PauseOutput(bool freeResources)
{
    if (!m_open)
        return;

	if (freeResources)
	{
		StopAudio();
	}
	else
	{
		LockMixer();
		m_shared.m_fillWithSilence = true;
		UnlockMixer();
	}
	
}

void SymbianOutput::ResumeOutput()
{
	if (!m_open)
		return;

	if (!m_shared.m_threadRunning)
	{
		StartAudio();
	}
	else
	{
		LockMixer();
		m_shared.m_fillWithSilence = false;
		UnlockMixer();
	}

}


void SymbianOutput::LockMixer()
{
	if (!m_shared.m_threadRunning)
		return;
	// wait for access to shared data
	m_shared.iMutex.Wait();
}

void SymbianOutput::UnlockMixer()
{
	if (!m_shared.m_threadRunning)
		return;
  // give access to shared data
  m_shared.iMutex.Signal();	
}

void SymbianOutput::FillBuffer(void *ptr, int numFrames)
{
	// Mix data
	UpdateStatus();
	FeedData( ptr, numFrames);
}


} // namespace audio
} // namespace cz

