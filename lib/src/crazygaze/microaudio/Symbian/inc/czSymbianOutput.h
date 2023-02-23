//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZ_SYMBIAN_OUTPUT_H_
#define _CZ_SYMBIAN_OUTPUT_H_

#include "../czPrivateDefs.h"
#include "../czSoundOutput.h"
#include "czAudioShared.h"

namespace cz
{
namespace audio
{


class SymbianOutput : public SoundOutput
{

public:
	SymbianOutput(cz::Core *parentObject);
	virtual ~SymbianOutput();

	virtual int Init(int maxActiveSounds, int mixSizeMs, bool stereo, bool bits16, int freq );

	// To be called from CMixerThread;
	void FillBuffer(void *ptr, int numFrames);

protected:
	virtual void LockMixer();
	virtual void UnlockMixer();
	
private:	

	
	virtual void PauseOutput(bool freeResources);
	virtual void ResumeOutput(void);

	int StartAudio();
	int StopAudio();
	

	bool m_isReady;
	bool m_paused;

private:
    TSharedData  m_shared;        // shared data with mixer thread
    //RThread       iMixerThread;   // handle to mixer thread
    //TBool         iPaused;        // paused flag	
    
    volatile bool m_open;
    RThread m_thread;
};

} // namespace audio
} // namespace cz

#endif
