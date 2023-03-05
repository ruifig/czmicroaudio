#pragma once

#include <crazygaze/microaudio/Config.h>
#include <crazygaze/microaudio/output/BaseOutput.h>
#include <crazygaze/microaudio/Memory.h>

namespace cz::microaudio
{

// Using Pimpl pattern do we don't need add Windows's Mmsystem.h to the global scope
struct Win32WaveOutOutputImpl;

class Win32WaveOutOutput : public BaseOutput
{
public:

	struct Options
	{
	};

	Win32WaveOutOutput();
	virtual ~Win32WaveOutOutput();

	virtual Error begin(AudioSource& source, const OutputConfig* cfg) override;
	virtual Error stop() override;
	virtual uint32_t getSampleRate() const override;
	virtual uint8_t getBitDepth() const override;
	virtual uint8_t getNumChannels() const override;
private:

	UniquePtr<Win32WaveOutOutputImpl> m_impl = nullptr;
};

} // namespace cz::microaudio

