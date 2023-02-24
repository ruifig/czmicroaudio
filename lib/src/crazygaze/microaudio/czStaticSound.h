//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZSTATICSOUND_H
#define _CZSTATICSOUND_H


/** \file */

#include "czPlayerPrivateDefs.h"
#include "czObject.h"
#include "czAudio.h"

namespace cz
{
namespace audio
{


// Number of extra frames in start and end to allocate for sounds
enum
{
	SOUND_SAFETYAREA=4
};

/*
7 6 5 4 3 2 1 0
| | | | ^- ok/off
| | | |--> 16bits (on/off)
| | |----> stereo (on/off)                        
| |------> signed (on/off)
|--------> adpcm (on/off)
*/
enum SoundTypeFlags
{
	SOUND_8BITS=0x00000008,
	SOUND_16BITS=0x00000010,
	SOUND_MONO=0x00000020,
	SOUND_STEREO=0x00000040,
	SOUND_UNSIGNED=0x00000080,
	SOUND_SIGNED=0x00000100,
	SOUND_ADPCM=0x00000200
};

/*
NOTES:

- A sample is considered the smallest part of channel.
- A frame is considered the smallest output of a sound.

Example for a 8 bits MONO sound:
	- 1 sample has 8 bits
	- 1 frame has 1 sample, totaling 8 bits
Example for a 8 bits STEREO sound.
	- 1 sample is 8 bits
	- 1 frame has 2 samples, totaling 16 bits (8+8).
*/


/******************* StaticSound class *************************/

//! Base class for static sounds that are stored in memory
/*!
 * Use \link czPlayer::LoadWAV \endlink to load WAV files
 */
class StaticSound : public ::cz::Object {

public:
	StaticSound(::cz::Core *parentObject);
	virtual ~StaticSound();

	//! Initializes the internal sound buffer
	/*!
	* This function creates the internal buffer needed for every sound.
	*
	* \param form Format
	* \param frames Size of the sound in frames
	*
	* \return ERR_OK on success, other on error.
	*
	*/
	int Set(int form, int frames);


	//! Changes the sign of the sound data
	/*!
	* \return ERR_OK on success, other on error.
	*/
	int ChangeSign(void);

	//! Returns the size in bytes of the sound data
	int GetLength(void)
	{
		return(m_length);
	};
	
	//! Returns the size in frames of the sound data
	int GetNumFrames(void)
	{
		return m_frames;
	}

	//! Tells if the sound data is in 16bits format.
	bool Is16Bits(void)
	{
		return ((m_format&SOUND_16BITS)!=0);
	}
	//! Tells if the sound data is in stereo format.
	bool IsStereo(void)
	{
		return ((m_format&SOUND_STEREO)!=0);
	}
	//! Tells if the sound data is signed.
	bool IsSigned(void)
	{
		return ((m_format&SOUND_SIGNED)!=0);
	}
	//! Tells if the sound is in adpcm format	
	bool IsADPCM(void)
	{
		return ((m_format&SOUND_ADPCM)!=0);
	}

	//! Returns the frequency of the sound
	int GetFrequency(void)
	{
		return(m_frequency);
	}
	//! Returns the default volume
	int GetVolume(void)
	{
		return m_vol;
	}
	
	//! Returns the default panning
	int GetPanning(void)
	{
		return m_pan;
	}
	
	//!
	/* Fills the passed pointers with information about the loop mode of the sound
	 * \param loopMode Loop mode of the sound. \link SOUND_LOOP_OFF \endlink, \link SOUND_LOOP_NORMAL \endlink, or \link SOUND_LOOP_BIDI \endlink
	 * \param loopStart Start of the loop. Expressed in frames
	 * \param loopLength Length of the loop, starting at loopStart. This value is expressed in frames.
	 */
	void GetLoopMode(int *loopMode, int *loopStart, int *loopLength);



	//! Returns a pointer to a give frame position
	/*!
	* \param frame Desired frame position.
	* \sa \link GetPtr \endlink
	*/
	void *GetPtrToFrame(int frame)
	{
		return (m_dptr+((frame*m_framesize)/8));
	}

	//! Returns the pointer to the beginning of the sound data (first frame)
	/*!
	* \sa \link GetPtrToFrame \endlink
	*/
	void *GetPtr(void)
	{
		return(m_dptr);
	}

	int SetLoopMode(int loopMode, int loopStart=0, int loopLength=0);

	/**  Set default parameters to use when playing the sound
	 * \param freq Frequency in hertz. -1 to ignore
	 * \param vol Volume. From 0 to AUDIO_VOL_MAX . -1 to ignore
	 * \param pan Panning. From CZPAN_lEFT to AUDIO_PAN_RIGHT, or AUDIO_PAN_MIDDLE. -1 to ignore
	 * \return ERR_OK on success, other on error
	 */
	int SetDefaults(int freq=-1, int vol=-1, int pan=-1);

	//! Calculate the frame size of the given format
	/*!
	 * \param format Format of the sound
	 * \return Size of frame, in bits. ex : (16bits,stereo)= 32bits
	 */
	static int CalcFrameSize(int format);

	void SmoothLoop(void);

//protected:

	//! Returns the frame size, in bits
	int GetFrameSize()
	{
		return m_framesize;
	}

	//! Returns the frame size, in bytes
	int GetFrameSizeBytes()
	{
		return m_framesize/8;
	}

	void SetToSilence();

private:

	int m_format;

	int m_frequency;
	uint8_t m_vol;
	uint8_t m_pan;

	int m_length;	// length of sound in bytes
	int m_frames;  // nº of frames
	int m_framesize; // frame size in BITS
	uint8_t *m_startptr; // used to free memory
	int m_allocatedSize; // total allocated size in bytes, accounting for the safety bounds
	uint8_t *m_dptr; // pointer to samples

	int m_loopStart;
	int m_loopLength;

};

} // namespace audio
} // namespace cz


#endif
