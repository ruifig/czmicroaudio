//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czWin32WaveOutOutput.h"
#include <thread>
#include <mutex>
#include <windows.h>
#include <Mmsystem.h>

namespace cz
{
namespace audio
{

class Win32WaveOutOutputImpl : public ::cz::Object
{

public:
	Win32WaveOutOutputImpl(Win32WaveOutOutput* outer);
	virtual ~Win32WaveOutOutputImpl();
	int Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq );
	void PauseOutput(bool freeResources);
	void ResumeOutput(void);

	int writeBlock(void);
	void LockMixer();
	void UnlockMixer();
private:

	Win32WaveOutOutput* m_outer;
	HWAVEOUT m_hWaveOut; /* device handle */
	std::mutex m_waveOutMutex;
	std::recursive_mutex m_mixerMutex;
	std::condition_variable m_processCond;
	volatile bool m_finish;
	volatile uint32_t m_msgCounter;
	std::thread m_workerThread;

	WAVEHDR*         m_waveBlocks;
	volatile int     m_waveFreeBlockCount;
	int              m_waveCurrentBlock;
	int m_numFramesInBlock;

	static void RunWorkerThread(volatile bool &started, Win32WaveOutOutputImpl* sndOut);

	int m_waveBlockSize; // Block size in bytes

	static void CALLBACK waveOutProc( HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};

//#define CZWIN32WAVEOUT_BLOCK_SIZE 8820
#define CZWIN32WAVEOUT_BLOCK_COUNT 4

void Win32WaveOutOutputImpl::RunWorkerThread(volatile bool& started, Win32WaveOutOutputImpl* sndOut)
{
	started = true;
	uint32_t counter=sndOut->m_msgCounter;
	while(!sndOut->m_finish)
	{
		while(counter==sndOut->m_msgCounter && !sndOut->m_finish)
		{
			std::unique_lock<std::mutex> lk(sndOut->m_waveOutMutex);
			sndOut->m_processCond.wait(lk);
		}

		while(counter!=sndOut->m_msgCounter)
		{
			counter++;
			sndOut->writeBlock();
		}
	}
}

/*
void Win32OutputWorkerThread::Run()
{
	int msgID;
	unsigned wparam, lparam;
	m_isrunning = true;
	while (GetUserMessage(&msgID, &wparam, &lparam))
	{
		if (msgID==1)
		{
			m_sndOut->writeBlock();
		}
	}
	m_isrunning = false;
}
*/

/*
char* WaveOutErrorStr(MMRESULT err)
{
	static char buf[500];
	waveOutGetErrorText(err,buf,sizeof(buf)-1);
	return buf;
}
*/

void CALLBACK Win32WaveOutOutputImpl::waveOutProc( HWAVEOUT CZUNUSED(hWaveOut), UINT uMsg, DWORD dwInstance, DWORD CZUNUSED(dwParam1), DWORD CZUNUSED(dwParam2))
{

	if(uMsg==WOM_DONE){
		Win32WaveOutOutputImpl* obj = (Win32WaveOutOutputImpl*) dwInstance ;
		obj->m_waveOutMutex.lock();
		obj->m_waveFreeBlockCount++;
		obj->m_msgCounter++;
		obj->m_waveOutMutex.unlock();		
		obj->m_processCond.notify_one();
	}

}


void Win32WaveOutOutputImpl::LockMixer(){
	m_mixerMutex.lock();
}
void Win32WaveOutOutputImpl::UnlockMixer(){
	m_mixerMutex.unlock();
}

int Win32WaveOutOutputImpl::writeBlock(void)
{

	WAVEHDR* current;
	MMRESULT err;

	if (m_waveFreeBlockCount<=0) CZERROR(ERR_CANTRUN);

	current = &m_waveBlocks[m_waveCurrentBlock];

	/*  first make sure the header we're going to use is unprepared	*/
	if(current->dwFlags & WHDR_PREPARED)
		if ((err=waveOutUnprepareHeader(m_hWaveOut, current, sizeof(WAVEHDR)))!=MMSYSERR_NOERROR){
			CZLOG(LOG_ERROR, "Error calling waveOutUnprepareHeader : Error %d\n", (int)err);
			CZERROR(ERR_BADAPICALL);
		}

	m_outer->UpdateStatus();
	m_outer->FeedData(current->lpData, m_numFramesInBlock);
	current->dwBufferLength = m_waveBlockSize;
	current->dwUser = m_waveBlockSize;
	current->dwFlags=0;

	if ((err=waveOutPrepareHeader(m_hWaveOut, current, sizeof(WAVEHDR)))!=MMSYSERR_NOERROR){
		CZLOG(LOG_ERROR, "Error calling waveOutPrepareHeader : Error %d\n", (int)err);
		CZERROR(ERR_BADAPICALL);
	}

	if ((err=waveOutWrite(m_hWaveOut, current, sizeof(WAVEHDR)))!=MMSYSERR_NOERROR){
		CZLOG(LOG_ERROR, "Error calling waveOutWrite : Error %d\n", (int)err);
		CZERROR(ERR_BADAPICALL);
	}

	/* point to the next block */
	m_waveCurrentBlock++;
	m_waveCurrentBlock %= CZWIN32WAVEOUT_BLOCK_COUNT;
	{
		m_waveOutMutex.lock();
		m_waveFreeBlockCount--;
		m_waveOutMutex.unlock();
	}

	return ERR_OK;
}

Win32WaveOutOutputImpl::Win32WaveOutOutputImpl(Win32WaveOutOutput* outer)
	: ::cz::Object(outer->m_core)
{
	PROFILE();
	m_outer = outer;
	m_hWaveOut = NULL;
	m_waveBlocks = NULL;
	m_msgCounter = 0;
	m_finish = false;
}

Win32WaveOutOutputImpl::~Win32WaveOutOutputImpl()
{
	PROFILE();

	MMRESULT ret;

	if (m_hWaveOut!=NULL){

		// Wait for all blocks to play
		m_finish = true;
		m_processCond.notify_one();
		m_workerThread.join();

		if ((ret=waveOutReset(m_hWaveOut))!=MMSYSERR_NOERROR){
			CZLOG(LOG_ERROR, "ERROR calling waveOutReset : Error %d\n", (int)ret);
		}

		/*unprepare any blocks that are still prepared */
		if ((m_hWaveOut!=NULL)&&(m_waveBlocks!=NULL)) {
			for(int i = 0; i < CZWIN32WAVEOUT_BLOCK_COUNT; i++)
				if(m_waveBlocks[i].dwFlags & WHDR_PREPARED)
					if ((ret=waveOutUnprepareHeader(m_hWaveOut, &m_waveBlocks[i], sizeof(WAVEHDR)))!=MMSYSERR_NOERROR)
						CZLOG(LOG_ERROR, "ERROR calling waveOutUnprepareHeader : Error %d\n", (int)ret);;
		}
	
		if ((ret=waveOutClose(m_hWaveOut))!=MMSYSERR_NOERROR){
			CZLOG(LOG_ERROR, "ERROR calling waveOutClose : %d\n", (int)ret);
		}
	}

	// delete as (unsigned char*), because it was allocated as such
	if (m_waveBlocks!=NULL) CZFREE((unsigned char*)m_waveBlocks);

}


int Win32WaveOutOutputImpl::Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq )
{
	MMRESULT err;
	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = freq;
	wfx.wBitsPerSample = (bits16) ? 16 : 8;
	wfx.nChannels      = (stereo) ? 2 : 1;
	wfx.cbSize          = 0; /* size of _extra_ info */
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nBlockAlign     = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	// Calculate the size of the block in bytes, for waveOut functions, based on the wanted latency
	m_waveBlockSize = ((wfx.nAvgBytesPerSec*mixSizeMs)/1000) / CZWIN32WAVEOUT_BLOCK_COUNT;
	// Round up to be multiple of nBlockAligh
	m_waveBlockSize += m_waveBlockSize % wfx.nBlockAlign;

	// Allocate the blocks necessary 
	{
		unsigned char* buffer;
		int i;
		DWORD totalBufferSize = (m_waveBlockSize + sizeof(WAVEHDR)) * CZWIN32WAVEOUT_BLOCK_COUNT;

		/* allocate memory for all the blocks in one go */
		if((buffer = (unsigned char*) CZALLOC(sizeof(unsigned char)*totalBufferSize))==NULL) CZERROR(ERR_NOMEM);
		memset(buffer,0, totalBufferSize);

		/* setup the headers */
		m_waveBlocks = (WAVEHDR*)buffer;
		buffer += sizeof(WAVEHDR) * CZWIN32WAVEOUT_BLOCK_COUNT;
		for(i = 0; i < CZWIN32WAVEOUT_BLOCK_COUNT; i++) {
				m_waveBlocks[i].dwBufferLength = m_waveBlockSize;
				m_waveBlocks[i].lpData = (char*)buffer;
				buffer += m_waveBlockSize;
			}
	}
	
	// Create the sound update thread
	m_waveFreeBlockCount = 0;
	m_waveCurrentBlock   = 0;
	volatile bool started=false;
	m_workerThread = std::thread(RunWorkerThread, std::ref(started), this);
	bool res = SetThreadPriority(m_workerThread.native_handle(), THREAD_PRIORITY_HIGHEST);

	// wait until it starts
	while(!started)
	{
		Sleep(1);
	}

	CZLOG(LOG_INFO, "workerTread started...\n");

	/* Open the default wave device */
	if((err=waveOutOpen( &m_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION)) != MMSYSERR_NOERROR) {
		CZLOG(LOG_ERROR, "Error calling waveOutOpen : Error %d\n", (int)err);
		CZERROR(ERR_BADAPICALL);
	}
	CZLOG(LOG_INFO, "waveOutOpen call successful.\n");
	if ((err=waveOutPause(m_hWaveOut))!=MMSYSERR_NOERROR){
		CZLOG(LOG_ERROR, "Error calling waveOutPause : Error %d\n", (int)err);
		CZERROR(ERR_BADAPICALL);
	}

	m_numFramesInBlock = m_waveBlockSize / wfx.nBlockAlign;
	// Init the mixer stuff in the parent class
	int ret = m_outer->InitSoftwareMixerOutput(maxActiveSounds, m_numFramesInBlock, stereo, bits16, freq);
	if (ret!=ERR_OK) CZERROR(ret);

	for(int i = 0; i < CZWIN32WAVEOUT_BLOCK_COUNT; i++)
	{
		WAVEHDR* current = &m_waveBlocks[i];
		current->dwBufferLength = m_waveBlockSize;
		current->dwUser = m_waveBlockSize;

		if ((err=waveOutPrepareHeader(m_hWaveOut, current, sizeof(WAVEHDR)))!=MMSYSERR_NOERROR){
			CZLOG(LOG_ERROR, "Error calling waveOutPrepareHeader : Error %d\n", (int)err);
			CZERROR(ERR_BADAPICALL);
		}
		if ((err=waveOutWrite(m_hWaveOut, current, sizeof(WAVEHDR)))!=MMSYSERR_NOERROR){
			CZLOG(LOG_ERROR, "Error calling waveOutWrite : Error %d\n", (int)err);
			CZERROR(ERR_BADAPICALL);
		}
	}
	CZLOG(LOG_INFO, "Initial block filling complete...\n");

	waveOutRestart(m_hWaveOut);

	Sleep(500);
	return ERR_OK;
}


void Win32WaveOutOutputImpl::PauseOutput(bool freeResources)
{
	waveOutPause(m_hWaveOut);
}

void Win32WaveOutOutputImpl::ResumeOutput()
{
	waveOutRestart(m_hWaveOut);
}

//////////////////////////////////////////////////////////////////////////
// Win32WaveOutOutputImpl
//////////////////////////////////////////////////////////////////////////


Win32WaveOutOutput::Win32WaveOutOutput(::cz::Core *parentObject)
	: SoundOutput(parentObject)
{
	m_impl = CZNEW(Win32WaveOutOutputImpl)(this);
}

Win32WaveOutOutput::~Win32WaveOutOutput()
{
	CZDELETE(m_impl);
}

int Win32WaveOutOutput::Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq )
{
	SoundOutput::Init(maxActiveSounds, mixSizeMs, stereo, bits16, freq);
	return m_impl->Init(maxActiveSounds, mixSizeMs, stereo, bits16, freq);
}

void Win32WaveOutOutput::PauseOutput(bool freeResources)
{
	m_impl->PauseOutput(freeResources);
}

void Win32WaveOutOutput::ResumeOutput(void)
{
	m_impl->ResumeOutput();
}

void Win32WaveOutOutput::LockMixer()
{
	m_impl->LockMixer();
}

void Win32WaveOutOutput::UnlockMixer()
{
	m_impl->UnlockMixer();
}

} // namespace audio
} // namespace cz

