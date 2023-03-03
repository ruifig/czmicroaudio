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

#pragma once

#include <crazygaze/microaudio/PlayerPrivateDefs.h>
#include <crazygaze/microaudio/SoundOutput.h>

namespace cz::microaudio
{

// Using Pimpl pattern do we don't need add Windows's Mmsystem.h to the global scope
struct Win32WaveOutOutputImpl;

class Win32WaveOutOutput : public SoundOutput
{

public:

	Win32WaveOutOutput();
	virtual ~Win32WaveOutOutput();
	virtual int Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq ) override;
	virtual void PauseOutput(bool freeResources) override;
	virtual void ResumeOutput(void) override;

	using SoundOutput::Init;
	using SoundOutput::FeedData;
	using SoundOutput::UpdateStatus;
	using SoundOutput::InitSoftwareMixerOutput;

private:
	virtual void LockMixer() override;
	virtual void UnlockMixer() override;

	Win32WaveOutOutputImpl* m_impl = nullptr;
};

} // namespace cz::microaudio

