//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/Config.h>
#if CZMICROAUDIO_PLATFORM_WINDOWS

#include <crazygaze/microaudio/output/win32/Win32WaveOutOutput.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/Memory.h>
#include <crazygaze/microaudio/AudioSource.h>

#include <thread>
#include <mutex>
#include <windows.h>
#include <Mmsystem.h>

namespace cz::microaudio
{

class Win32WaveOutOutputImpl
{

public:
	Win32WaveOutOutputImpl(Win32WaveOutOutput* outer);
	virtual ~Win32WaveOutOutputImpl();
	Error begin(AudioSource& source, const OutputConfig* cfg);
	Error stop();

	uint32_t getSampleRate() const
	{
		return m_cfg.sampleRate;
	}

	uint8_t getBitDepth() const
	{
		return m_cfg.bitDepth;
	}

	uint8_t getNumChannels() const
	{
		return m_cfg.numChannels;
	}

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

	OutputConfig m_cfg;
	AudioSource* m_source = nullptr;

	Error writeBlock(void);
	static void runEntry(volatile bool &started, Win32WaveOutOutputImpl* sndOut);
	void run(volatile bool &started);

	int m_waveBlockSizeBytes; // Block size in bytes
	int m_waveBlockSizeFrames;

	static void CALLBACK waveOutProc( HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};

#define CZWIN32WAVEOUT_BLOCK_COUNT 4

Win32WaveOutOutputImpl::Win32WaveOutOutputImpl(Win32WaveOutOutput* outer)
{
	PROFILE();
	m_outer = outer;
	m_hWaveOut = nullptr;
	m_waveBlocks = nullptr;
	m_msgCounter = 0;
	m_finish = false;
}

Win32WaveOutOutputImpl::~Win32WaveOutOutputImpl()
{
	PROFILE();

	MMRESULT ret;

	if (m_hWaveOut != nullptr)
	{
		// Wait for all blocks to play
		m_finish = true;
		m_processCond.notify_one();
		m_workerThread.join();

		if ((ret=waveOutReset(m_hWaveOut))!=MMSYSERR_NOERROR){
			CZMICROAUDIO_LOG(LogLevel::Error, "ERROR calling waveOutReset : Error %d\n", (int)ret);
		}

		/*unprepare any blocks that are still prepared */
		if ((m_hWaveOut != nullptr) && (m_waveBlocks != nullptr))
		{
			for(int i = 0; i < CZWIN32WAVEOUT_BLOCK_COUNT; i++)
			{
				if(m_waveBlocks[i].dwFlags & WHDR_PREPARED)
				{
					if ((ret=waveOutUnprepareHeader(m_hWaveOut, &m_waveBlocks[i], sizeof(WAVEHDR)))!=MMSYSERR_NOERROR)
					{
						CZMICROAUDIO_LOG(LogLevel::Error, "ERROR calling waveOutUnprepareHeader : Error %d\n", (int)ret);
					}
				}
			}
		}
	
		if ((ret=waveOutClose(m_hWaveOut))!=MMSYSERR_NOERROR){
			CZMICROAUDIO_LOG(LogLevel::Error, "ERROR calling waveOutClose : %d\n", (int)ret);
		}
	}

	// delete as (unsigned char*), because it was allocated as such
	if (m_waveBlocks != nullptr)
	{
		CZMICROAUDIO_FREE((unsigned char*)m_waveBlocks);
	}
}

Error Win32WaveOutOutputImpl::begin(AudioSource& source, const OutputConfig* cfg)
{
	m_source = &source;

	m_cfg.sampleRate = (cfg && cfg->sampleRate != 0) ? cfg->sampleRate : 48000;
	m_cfg.bitDepth = (cfg && cfg->bitDepth != 0 && (cfg->bitDepth==8 || cfg->bitDepth==16)) ? cfg->bitDepth : 16;
	m_cfg.numChannels = (cfg && cfg->numChannels != 0 && (cfg->numChannels==1 || cfg->numChannels==2)) ? cfg->numChannels : 2;
	m_cfg.bufSizeMs = (cfg && cfg->bufSizeMs != 0) ? cfg->bufSizeMs : 200;

	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = m_cfg.sampleRate;
	wfx.wBitsPerSample = m_cfg.bitDepth;
	wfx.nChannels      = m_cfg.numChannels;
	wfx.cbSize          = 0; /* size of _extra_ info */
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nBlockAlign     = (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	// Calculate the size of the block in bytes, for waveOut functions, based on the wanted latency
	m_waveBlockSizeBytes = ((wfx.nAvgBytesPerSec * m_cfg.bufSizeMs) / 1000) / CZWIN32WAVEOUT_BLOCK_COUNT;
	// Round up to be multiple of nBlockAligh
	m_waveBlockSizeBytes += m_waveBlockSizeBytes % wfx.nBlockAlign;

	m_waveBlockSizeFrames = m_waveBlockSizeBytes / wfx.nBlockAlign;

	// Allocate the blocks necessary 
	{
		DWORD totalBufferSize = (m_waveBlockSizeBytes + sizeof(WAVEHDR)) * CZWIN32WAVEOUT_BLOCK_COUNT;

		/* allocate memory for all the blocks in one go */
		unsigned char* buffer = (unsigned char*)CZMICROAUDIO_ALLOC(sizeof(unsigned char) * totalBufferSize);
		if (buffer == nullptr)
		{
			CZMICROAUDIO_RET_ERROR(Error::OutOfMemory);
		}

		memset(buffer,0, totalBufferSize);

		/* setup the headers */
		m_waveBlocks = (WAVEHDR*)buffer;
		buffer += sizeof(WAVEHDR) * CZWIN32WAVEOUT_BLOCK_COUNT;
		for (int i = 0; i < CZWIN32WAVEOUT_BLOCK_COUNT; i++)
		{
			m_waveBlocks[i].dwBufferLength = m_waveBlockSizeBytes;
			m_waveBlocks[i].lpData = (char*)buffer;
			buffer += m_waveBlockSizeBytes;
		}
	}
	
	// Create the sound update thread
	m_waveFreeBlockCount = 0;
	m_waveCurrentBlock   = 0;
	volatile bool started=false;
	m_workerThread = std::thread(runEntry, std::ref(started), this);
	bool res = SetThreadPriority(m_workerThread.native_handle(), THREAD_PRIORITY_HIGHEST);

	// wait until it starts
	while(!started)
	{
		Sleep(1);
	}

	CZMICROAUDIO_LOG(LogLevel::Log, "workerTread started...\n");

	MMRESULT err = waveOutOpen( &m_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	/* Open the default wave device */
	if (err != MMSYSERR_NOERROR)
	{
		CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutOpen : Error %d\n", (int)err);
		CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
	}
	CZMICROAUDIO_LOG(LogLevel::Log, "waveOutOpen call successful.\n");

	if ((err=waveOutPause(m_hWaveOut))!=MMSYSERR_NOERROR)
	{
		CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutPause : Error %d\n", (int)err);
		CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
	}


	// #TODO Initialize mixer?
	// Init the mixer stuff in the parent class
	//int ret = m_outer->InitSoftwareMixerOutput(maxActiveSounds, m_waveBlockSizeFrames, stereo, bits16, freq);
	//if (ret != Error::Success)
	//{
	//	CZMICROAUDIO_RET_ERROR(static_cast<Error>(ret));
	//}

	for(int i = 0; i < CZWIN32WAVEOUT_BLOCK_COUNT; i++)
	{
		WAVEHDR* current = &m_waveBlocks[i];
		current->dwBufferLength = m_waveBlockSizeBytes;
		current->dwUser = m_waveBlockSizeBytes;

		if ((err = waveOutPrepareHeader(m_hWaveOut, current, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
		{
			CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutPrepareHeader : Error %d\n", (int)err);
			CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
		}

		if ((err = waveOutWrite(m_hWaveOut, current, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
		{
			CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutWrite : Error %d\n", (int)err);
			CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
		}
	}
	CZMICROAUDIO_LOG(LogLevel::Log, "Initial block filling complete...\n");

	CZMICROAUDIO_CHECK(waveOutRestart(m_hWaveOut) == MMSYSERR_NOERROR);

	//Sleep(500);
	return Error::Success;
}

Error Win32WaveOutOutputImpl::stop()
{
	// #TODO : Probably change stop() to be the opposite of begin, releasing all resources
	waveOutPause(m_hWaveOut);
	return Error::Success;
}

void Win32WaveOutOutputImpl::run(volatile bool &started)
{
	started = true;
	uint32_t counter=m_msgCounter;
	while(!m_finish)
	{
		while(counter==m_msgCounter && !m_finish)
		{
			std::unique_lock<std::mutex> lk(m_waveOutMutex);
			m_processCond.wait(lk);
		}

		while(counter!=m_msgCounter)
		{
			counter++;
			writeBlock();
		}
	}
}

void Win32WaveOutOutputImpl::runEntry(volatile bool& started, Win32WaveOutOutputImpl* _this)
{
	_this->run(started);
}

void CALLBACK Win32WaveOutOutputImpl::waveOutProc( HWAVEOUT /*hWaveOut*/, UINT uMsg, DWORD dwInstance, DWORD /*dwParam1*/, DWORD /*dwParam2*/)
{
	if(uMsg==WOM_DONE)
	{
		Win32WaveOutOutputImpl* obj = (Win32WaveOutOutputImpl*) dwInstance ;
		obj->m_waveOutMutex.lock();
		obj->m_waveFreeBlockCount++;
		obj->m_msgCounter++;
		obj->m_waveOutMutex.unlock();		
		obj->m_processCond.notify_one();
	}
}

Error Win32WaveOutOutputImpl::writeBlock(void)
{
	WAVEHDR* current;

	if (m_waveFreeBlockCount<=0)
	{
		CZMICROAUDIO_RET_ERROR(Error::CantRun);
	}

	current = &m_waveBlocks[m_waveCurrentBlock];

	/*  first make sure the header we're going to use is unprepared	*/
	if(current->dwFlags & WHDR_PREPARED)
	{
		if (MMRESULT err = waveOutUnprepareHeader(m_hWaveOut, current, sizeof(WAVEHDR)); err != MMSYSERR_NOERROR)
		{
			CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutUnprepareHeader : Error %d\n", (int)err);
			CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
		}
	}

	m_source->get(current->lpData, m_waveBlockSizeFrames);
	current->dwBufferLength = m_waveBlockSizeBytes;
	current->dwUser = m_waveBlockSizeBytes;
	current->dwFlags=0;

	if (MMRESULT err=waveOutPrepareHeader(m_hWaveOut, current, sizeof(WAVEHDR)); err!=MMSYSERR_NOERROR)
	{
		CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutPrepareHeader : Error %d\n", (int)err);
		CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
	}

	if (MMRESULT err = waveOutWrite(m_hWaveOut, current, sizeof(WAVEHDR)); err != MMSYSERR_NOERROR)
	{
		CZMICROAUDIO_LOG(LogLevel::Error, "Error calling waveOutWrite : Error %d\n", (int)err);
		CZMICROAUDIO_RET_ERROR(Error::BadAPICall);
	}

	/* point to the next block */
	m_waveCurrentBlock++;
	m_waveCurrentBlock %= CZWIN32WAVEOUT_BLOCK_COUNT;
	{
		m_waveOutMutex.lock();
		m_waveFreeBlockCount--;
		m_waveOutMutex.unlock();
	}

	return Error::Success;
}

//////////////////////////////////////////////////////////////////////////
// Win32WaveOutOutput
//////////////////////////////////////////////////////////////////////////


Win32WaveOutOutput::Win32WaveOutOutput()
{
	m_impl = makeUnique(Win32WaveOutOutputImpl, this);
}

Win32WaveOutOutput::~Win32WaveOutOutput()
{
}

Error Win32WaveOutOutput::begin(AudioSource& source, const OutputConfig* cfg)
{
	return m_impl->begin(source, cfg);
}

Error Win32WaveOutOutput::stop()
{
	return m_impl->stop();
}

uint32_t Win32WaveOutOutput::getSampleRate() const
{
	return m_impl->getSampleRate();
}

uint8_t Win32WaveOutOutput::getBitDepth() const
{
	return m_impl->getBitDepth();
}

uint8_t Win32WaveOutOutput::getNumChannels() const
{
	return m_impl->getNumChannels();
}

} // namespace cz::microaudio

#endif // if CZMICROAUDIO_PLATFORM_WINDOWS
