//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czSampler.h"
#include "czStandardC.h"

namespace cz
{
namespace audio
{


//////////////////////////////////////////////////////////////////////////
// SoundBuffer
//////////////////////////////////////////////////////////////////////////

SoundBuffer::SoundBuffer(::cz::Core *parentObject) : ::cz::Object(parentObject)
{
	m_type = kSOUNDBUFFER_NONE;
	m_numChannels = 0;
	m_numFrames = 0;
	m_allocatedPtr = 0;
}

SoundBuffer::~SoundBuffer()
{
	if (m_allocatedPtr)
		CZFREE(m_allocatedPtr);
}

bool SoundBuffer::Init(int numFrames, int numChannels, SoundBufferType type)
{
	CZASSERT(numChannels<kMAX_CHANNELS);
	CZASSERT(m_allocatedPtr==NULL);
	CZASSERT(type!=kSOUNDBUFFER_NONE);

	if (numChannels>kMAX_CHANNELS || m_allocatedPtr)
		return false;


	m_type = type;
	m_numChannels = numChannels;
	m_numFrames = numFrames;
	int bytes = SizeOf(type)*numFrames*numChannels;
	m_allocatedPtr = static_cast<u8*>(CZALLOC(bytes));

	return true;
}

int SoundBuffer::SizeOf(int numFrames)
{
	switch(m_type)
	{
	case kSOUNDBUFFER_32FIXED:
		return numFrames*4;
	default:
		CZASSERT(0);
		return 0;
	}
}

//! Returns a pointer to a channel
void* SoundBuffer::GetPtrToSample(int channel, int sampleOffset)
{
	CZASSERT(m_allocatedPtr!=NULL);
	CZASSERT(channel<m_numChannels);
	CZASSERT(sampleOffset<m_numFrames);

	int samplesize = SizeOf(1);
	u8* p = m_allocatedPtr + (channel*(samplesize*m_numFrames)) + sampleOffset;
	return p;
}


} // namespace audio
} // namespace cz
