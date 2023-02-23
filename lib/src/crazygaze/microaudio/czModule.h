//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Base class for Modules (MOD, IT, etc)
//

#ifndef _CZMODULE_H_
#define _CZMODULE_H_

#include "czPlayerPrivateDefs.h"
#include "czObject.h"

// forward declarations
namespace cz
{
	namespace io
	{
		class File;
	}
}

namespace cz
{
namespace audio
{

// forward declarations
class SoundOutput;
class Mixer;

//! Base class for Module sounds. eg : MOD files
/*!
 * There is little need to mess directly with objects of this class. Basically, you can use the method
 * \link czPlayer::LoadModule \endlink to load the Module, and then use the methods provided by the \link czSoundOutput \endlink
 * class to play and stop.
 * \sa \link czPlayer::LoadModule \endlink
 * \sa \link czSoundOutput::PlayModule \endlink
 */
class Module : public ::cz::Object
{


public:
	Module(::cz::Core *parentObject);
	virtual ~Module();

	// Load the song
	virtual int Init(::cz::io::File *in) = 0;

	int GetBPM(void)
	{
		return m_bpm;
	}
	bool ReachedEnd()
	{
		return m_reachedEnd;
	}
//	czMixer* GetMixer(void) { return m_mixer; }

	//virtual int GetFirstMixerChannel(void) = 0;
	virtual int GetChannels(void)=0;
	virtual int GetPosition(void)=0;
	virtual int GetOrders(void)=0;
	virtual int GetPattern(void)=0;
	virtual int GetPatternRows(void)=0;
	virtual int GetRow(void)=0;
	virtual int GetSpeed(void)=0;
	virtual int SetMasterVolume(u8 vol) = 0;

protected :
	friend class SoundOutput;
	virtual void DoTick(void) = 0;

	//!
	/*!
	 * \param firstOrder - Where to start playing
	 * \param lastOrder - Where to finish playing, or -1 to play to the end
	 */
	virtual int Start(Mixer *mixer, int firstOrder, int lastOrder, bool loop, u8 volume=20)=0;
	virtual int Stop(void)=0;
	virtual int Pause(void)=0;
	virtual int Resume(void)=0;

	bool m_loop; // Tells if the song should loop
	bool m_reachedEnd; // Tells if the song has reached end

	int m_bpm; // BPM
	Mixer *m_mixer; // Mixer object to use.

};


} // namespace cz
} // namespace cz

#endif

