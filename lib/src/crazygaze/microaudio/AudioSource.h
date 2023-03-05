#pragma once

namespace cz::microaudio
{

class AudioSource
{
public:
	virtual uint32_t get(void* ptr, uint32_t numFrames)
	{
		return numFrames;
	}
};

} // cz::microaudio

