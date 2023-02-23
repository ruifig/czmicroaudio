//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czOpenALOutput.h"
#include "../czStandardC.h"

#include <al.h>
#include <alc.h>

namespace cz
{
namespace audio
{

// The reason we need a worker thread for OpenAL is because OpenAL
// doesn't provide any callbacks, so we need to poll to check what
// buffers are done

void OpenALOutput::RunWorkerThread(int sleeptime, volatile bool& started, OpenALOutput* sndOut)
{
	started = true;
	while(!sndOut->m_finish)
	{
		sndOut->CheckBuffers();
		::cz::this_thread::sleep_ms(sleeptime);
	}

}

OpenALOutput::OpenALOutput(Core* parentObject)
: SoundOutput(parentObject)
{
	m_device = NULL;
	m_context = NULL;
	m_bufsize = 0;
	m_mixbuffer = 0;
	m_mixBufferNumFrames = 0;
	m_uiSource = 0;	
	m_finish = false;
}

OpenALOutput::~OpenALOutput(void)
{

	// Wait for all blocks to play
	m_finish = true;
	m_workerThread.join();

	if (m_context)
	{
		if (alIsSource(m_uiSource))
		{
			alSourceStop(m_uiSource);
			alSourcei(m_uiSource, AL_BUFFER, 0);
			alDeleteSources(1, &m_uiSource);
		}

		for(int i=0; i<kNumBuffers; i++)
		{
			if (alIsBuffer(m_uiBuffers[i]))
				alDeleteBuffers(1, &m_uiBuffers[i]);
		}

		alcMakeContextCurrent(0);
		alcDestroyContext(m_context);
	}

	if (m_device)
	{
		alcCloseDevice(m_device);
	}

	if (m_mixbuffer)
	{
		CZFREE(m_mixbuffer);
	}
}

int OpenALOutput::Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq )
{
	SoundOutput::Init(maxActiveSounds, mixSizeMs, stereo, bits16, freq);

	ALenum error;

	alGetError(); // clear current error state
	m_device = alcOpenDevice(NULL); // Select the preferred device
	if (!m_device)
	{
		CZLOG(LOG_ERROR, "Error opening OpenAL device: Error %d\n", alGetError());
		CZERROR(ERR_BADAPICALL);
	}
	CZLOG(LOG_INFO, "OpenAL device opened succesfully.\n");

	alGetError(); // clear current error state
	m_context = alcCreateContext(m_device, NULL);
	if (!m_context)
	{
		CZLOG(LOG_ERROR, "Error creating OpenAL device context: Error %d\n", alGetError());
		CZERROR(ERR_BADAPICALL);
	}
	alcMakeContextCurrent(m_context);

	alGetError(); // clear current error state
	alGenBuffers(kNumBuffers, m_uiBuffers);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		CZLOG(LOG_ERROR, "Error creating OpenAL buffers. Error %d\n", error);
		CZERROR(ERR_BADAPICALL);
	}

	// Generate a Source to playback the Buffers
	alGetError(); // clear current error state
	alGenSources(1, &m_uiSource);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		CZLOG(LOG_ERROR, "Error creating OpenAL source. Error %d\n", error);
		CZERROR(ERR_BADAPICALL);
	}


	m_mixBufferNumFrames = (freq*mixSizeMs)/1000;
	int framesize = 1;
	if (stereo) framesize *=2;
	if (bits16) framesize *=2;
	m_bufsize = m_mixBufferNumFrames*framesize;
	m_mixbuffer = CZALLOC(m_bufsize);
	if (!m_mixbuffer)
	{
		CZERROR(ERR_NOMEM);
	}
	memset(m_mixbuffer, 0, m_bufsize);

	// Init the mixer stuff in the parent class
	int ret = InitSoftwareMixerOutput(maxActiveSounds, m_mixBufferNumFrames, stereo, bits16, freq);
	if (ret!=ERR_OK)
		CZERROR(ret);

	if (stereo)
		m_format = (bits16) ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
	else
		m_format = (bits16) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;


	// Fill all buffers with data
	for(int i=0; i<kNumBuffers; i++)
	{
		alGetError(); // clear current error state
		alBufferData(m_uiBuffers[i], m_format, m_mixbuffer, m_bufsize, freq);
		if ((error = alGetError()) != AL_NO_ERROR)
		{
			CZLOG(LOG_ERROR, "Error filling OpenAL buffers. Error %d\n", error);
			CZERROR(ERR_BADAPICALL);
		}
	}

	// Queue buffers
	alGetError(); // clear current error state
	alSourceQueueBuffers(m_uiSource, kNumBuffers, m_uiBuffers);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		CZLOG(LOG_ERROR, "Error queuing OpenAL buffers. Error %d\n", error);
		CZERROR(ERR_BADAPICALL);
	}

	// Create mixer thread
	// This is the thread the polls OpenAL to check when we can fill more buffers, and then calls the mixer
	// to fill the buffers
	volatile bool started = false;
	// Specify how often to poll OpenAL for buffer completion, so we can fill a new buffer as soon as possible
	// but without repeatedly polling.
	int sleeptime = mixSizeMs/2;
	m_workerThread.swap(::cz::thread(RunWorkerThread, sleeptime, ::cz::ref(started), this));
	while(!started)
		::cz::this_thread::sleep_ms(1);
	CZLOG(LOG_INFO, "workerTread started...\n");

	// Finally play the source
	alGetError(); // clear current error state
	alSourcePlay(m_uiSource);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		CZLOG(LOG_ERROR, "Error playing OpenAL source. Error %d\n", error);
		CZERROR(ERR_BADAPICALL);
	}

	return ERR_OK;
}

void OpenALOutput::CheckBuffers()
{
	ALint state=0;
	alGetSourcei(m_uiSource, AL_SOURCE_STATE, &state);
	assert(alGetError()==AL_NO_ERROR);
	if (state==AL_PAUSED)
	{
		return;
	}

	ALint buffersProcessed=0;
	alGetSourcei(m_uiSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
	assert(alGetError()==AL_NO_ERROR);

	ALint originalBufferProcessed= buffersProcessed;
	while(buffersProcessed--)
	{
		UpdateStatus();
		FeedData(m_mixbuffer, m_mixBufferNumFrames);
		ALuint uiBuffer=0;
		alSourceUnqueueBuffers(m_uiSource, 1, &uiBuffer);
		assert(alGetError()==AL_NO_ERROR);
		alBufferData(uiBuffer, m_format, m_mixbuffer, m_bufsize, m_mixer.GetMixFrequency());
		assert(alGetError()==AL_NO_ERROR);
		alSourceQueueBuffers(m_uiSource, 1, &uiBuffer);
		assert(alGetError()==AL_NO_ERROR);
	}

	// Restart the stream if necessary
	if (state != AL_PLAYING)
	{
		// If we get here, it means the source starved. This means probably audio is consuming too much CPU
		// and we can't feed the buffers fast enough, or the buffers size is too small.
		alSourcePlay(m_uiSource);
		assert(alGetError()==AL_NO_ERROR);
	}

}

void OpenALOutput::LockMixer()
{
	m_mixerMutex.lock();
}

void OpenALOutput::UnlockMixer()
{
	m_mixerMutex.unlock();
}

void OpenALOutput::PauseOutput(bool freeResources)
{
	alSourcePause(m_uiSource);
	assert(alGetError()==AL_NO_ERROR);
}

void OpenALOutput::ResumeOutput()
{
	alSourcePlay(m_uiSource);
	assert(alGetError()==AL_NO_ERROR);
}

} // namespace audio
} // namespace cz
