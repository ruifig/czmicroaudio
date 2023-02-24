//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZSAMPLER_H_
#define _CZSAMPLER_H_

/** \file */

#include "czPlayerPrivateDefs.h"
#include "czObject.h"
#include "czAudio.h"

namespace cz
{
namespace microaudio
{

	enum SoundBufferType
	{
		kSOUNDBUFFER_NONE,
		kSOUNDBUFFER_32FIXED // 32 bits fixed point
	};

	class SoundBuffer : public ::cz::Object
	{
	public:
		enum
		{
			kMAX_CHANNELS=2
		};
		SoundBuffer(::cz::Core *parentObject);
		virtual ~SoundBuffer();
		bool Init(int numFrames, int numChannels, SoundBufferType type);

		//! Returns a pointer to a channel
		void* GetPtrToSample(int channel, int sampleOffset);

		int SizeOf(int numFrames);

	private:
		SoundBufferType m_type;
		int m_numChannels;
		int m_numFrames;
		uint8_t* m_allocatedPtr; // Pointer to the memory we allocated
	};

	//! Structure that represents a request for audio data from an AudioSource interface
	struct AudioSourceRequest
	{
		SoundBuffer *pSndBuf; // pointer to the sound buffer we can write data to
		unsigned nMaxFrames; // Maximum of sound frames we can write to the buffer
		unsigned nValidFrames; // Number we actually produced. Should be <= nMaxFrames
	};

	// Interface for all audio sources
	class AudioSource
	{
	public:
		//! \brief Retrieves audio data from the audio source
		/*
		\param info Information about the request
			info.pSndBuf Sound buffer to use as destination
			info.nMaxFrames Should contain the number of frames requested (be carefull not to specify a value greater than what the sound buffer can handle)
			info.nValidFrames On function exit, will contain the number of frames actually retrieved
		\return Returns true if it still has more data, or false if finished
		*/
		virtual bool GetAudioData(AudioSourceRequest &info) = 0;
	};

	class Sampler : public ::cz::Object, public AudioSource
	{
	public:
		Sampler(::cz::Core *parentObject);
		virtual ~Sampler();

		struct State
		{
			int pos;
			int inc;
		} m_state;

	};

} // namespace microaudio
} // namespace cz

#endif

