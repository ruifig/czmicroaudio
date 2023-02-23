//
// czWin32WaveOutOutput.h
//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZWIN32WAVEOUTOUTPUT_H_
#define _CZWIN32WAVEOUTOUTPUT_H_


#include "../czPlayerPrivateDefs.h"
#include "../czSoundOutput.h"
#include "czThread.h"
#include "czMutex.h"
#include "czConditionVariable.h"

namespace cz
{
namespace audio
{

// forward declarations
//class Win32WaveOutOutput;

/*
class Win32OutputWorkerThread : public ::cz::Thread
{

friend class Win32WaveOutOutput; // czWin32WaveOutOutput can access protected members

public:
	Win32OutputWorkerThread() : m_sndOut(NULL), m_isrunning(false) {};
	virtual ~Win32OutputWorkerThread() {};
protected:
	Win32WaveOutOutput* m_sndOut;
	volatile bool m_isrunning;
	virtual void Run();
};
*/

class Win32WaveOutOutput : public SoundOutput
{

public:
	Win32WaveOutOutput(::cz::Core *parentObject);
	virtual ~Win32WaveOutOutput();

	virtual int Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq );

	virtual void PauseOutput(bool freeResources);
	virtual void ResumeOutput(void);

protected:
	int writeBlock(void);

	virtual void LockMixer();
	virtual void UnlockMixer();

private:

	HWAVEOUT m_hWaveOut; /* device handle */
	
	::cz::recursive_mutex m_waveOutMutex;
	::cz::recursive_mutex m_mixerMutex;
	::cz::condition_variable m_processCond;
	volatile bool m_finish;
	volatile u32 m_msgCounter;
	::cz::thread m_workerThread;

	WAVEHDR*         m_waveBlocks;
	volatile int     m_waveFreeBlockCount;
	int              m_waveCurrentBlock;
	int m_numFramesInBlock;

	static void RunWorkerThread(volatile bool &started, Win32WaveOutOutput* sndOut);

	int m_waveBlockSize; // Block size in bytes

	static void CALLBACK waveOutProc( HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};


} // namespace audio
} // namespace cz

#endif
