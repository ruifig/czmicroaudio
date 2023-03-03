//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#pragma once

#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/Audio.h>
#include <crazygaze/microaudio/File.h>
#include <crazygaze/microaudio/StaticSound.h>
#include <crazygaze/microaudio/MixerListener.h>
#include <crazygaze/microaudio/Memory.h>

#if CZMICROAUDIO_OGG_ENABLED
#include "ivorbiscodec.h"
#include "ivorbisfile.h"
#endif

namespace cz::microaudio
{

//! Only supports ogg vorbis at the moment
class StreamSound : public ::cz::microaudio::ChannelMixingListener
{

public:

	enum
	{
		kNUM_CHUNKS = 2
	};

	StreamSound();
	virtual ~StreamSound();

	// Load the stream
	//! \param in
	//	File to read from. Ownership is transfered to the StreamSound object. NOTE: Need to be allocated on the heap
	// \param workBufferNumFrames
	//	Size of the StaticSound used to by the mixer to feed the data
	virtual int Init(UniquePtr<File> in, int workBufferNumFrames);

	StaticSound* GetWorkBuffer()
	{
		return &m_snd;
	}

	// Returns the stream's frequency in hz
	int GetFrequency()
	{
		return m_snd.GetFrequency();
	}

	int GetVolume()
	{
		return m_snd.GetVolume();
	}

	int PrepareToPlay(bool loop);
	void FinishedPlaying();
	bool IsPlaying()
	{
		return m_isplaying;
	}

private:

	int Decode(void* dest, int bytes);

	//
	// ChannelMixingListener interface
	//
	virtual bool ChannelMix(int mixpos, int numframes) override;

	// We can't play a stream multiple times, so we need to detect if we're playing this stream already.
	// In the future, I can make it possible to play an ogg vorbis multiple times if it's loaded into memory,
	// by sharing the loaded memory
	bool m_isplaying;
	UniquePtr<File> m_in;
	StaticSound m_snd;

#if CZMICROAUDIO_OGG_ENABLED
	OggVorbis_File m_ogg;
#endif
	int m_lastPos; // Last played position within the working buffer
	
	// if this is -1, it means we're not finishing
	// If >0 it means the stream reached the end, and we're waiting to play "m_framesToMixBeforeFinish" frames
	// before stopping the mixer channel
	int m_framesToMixBeforeFinish; 

	bool m_loop;
	//StreamChunkReader m_jobs[kNUM_CHUNKS];
};

} // namespace cz::microaudio

