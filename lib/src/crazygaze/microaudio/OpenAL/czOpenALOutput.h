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

#ifndef __CZOPENALPOUTPUT__
#define __CZOPENALPOUTPUT__

#include "../czPlayerPrivateDefs.h"
#include "czThread.h"
#include "czMutex.h"
#include "czConditionVariable.h"
#include "../czSoundOutput.h"

#include <al.h>
#include <alc.h>

namespace cz
{
namespace audio
{


class OpenALOutput : public ::cz::audio::SoundOutput
{

friend class OpenALWorkerThread;

public:
	OpenALOutput(::cz::Core *parentObject);
	virtual ~OpenALOutput(void);

	virtual int Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq );

	virtual void PauseOutput(bool freeResources);
	virtual void ResumeOutput(void);

protected:
	virtual void LockMixer();
	virtual void UnlockMixer();

	void CheckBuffers();
private:

	::cz::recursive_mutex m_mixerMutex;
	volatile bool m_finish;
	::cz::thread m_workerThread;
	static void RunWorkerThread(int sleeptime, volatile bool& started, OpenALOutput* sndOut);

	enum
	{
		kNumBuffers = 4
	};
	ALuint m_uiSource;
	ALuint m_uiBuffers[kNumBuffers];
	ALCdevice* m_device;
	ALCcontext* m_context;
	int m_bufsize;
	void* m_mixbuffer;
	int m_mixBufferNumFrames;
	ALenum m_format;
};

} // namespace audio
} // namespace cz

#endif
