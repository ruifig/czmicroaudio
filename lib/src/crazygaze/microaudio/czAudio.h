//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
//
// --------------------------------------------------------------
//

#ifndef _CZ_AUDIO_H_
#define _CZ_AUDIO_H_

/*! \file
*/

#include "czPlayerConfig.h"
#include "czPlayer.h"

namespace cz
{

/*!
 * \brief Contains %audio functionality
 */
namespace microaudio
{

	/*!
	* \brief Handle for czPlayer sounds
	*
	* When playing a song or sound, you'll get a handler of this type.
	* With that handler, you can control its volume, frequency,etc.
	* \sa
	*		\link AudioPlayer \endlink,
	*		\link AudioPlayer::Play \endlink,
	*		\link AudioPlayer::PlayModule \endlink
	*/
	typedef int32_t HSOUND;

	/*!
	 * \brief Handle for sound data
	 *
	 * This is what you get when loading sounds.
	 * \sa
	 *		\link AudioPlayer::LoadWAV \endlink,
	 *		\link AudioPlayer::FreeStaticSound \endlink
	 */
	typedef void* HSOUNDDATA;

	/*!
	* \brief Handle for songs (tracked format. eg: MOD,IT)
	*
	* This is what you get when loading tracker songs.
	* \sa
	*		\link AudioPlayer::LoadModule \endlink,
	*		\link AudioPlayer::FreeModuleSound \endlink
	*/
	typedef void* HMODULEDATA;

	/*!
	 * \brief Handle for streams
	 *
	 * This is what you get when loading streams
	 * \sa
	 *		\link AudioPlayer::LoadStream \endlink
	 *		\link AudioPlayer::FreeStream \endlink
	 *
	 */
	typedef void* HSTREAMDATA;

	/*! Types of sound output drivers available
	 * \todo Implement AUDIO_DRIVER_NOSOUND, AUDIO_DRIVER_DSOUND, AUDIO_DRIVER_BADA, AUDIO_DRIVER_IPHONE, AUDIO_DRIVER_ANDROID
	 */
	enum AudioDriverType {
		AUDIO_DRIVER_DEFAULT=0,	//!< Default driver
		AUDIO_DRIVER_NOSOUND=1,	//!< void output. All commands work, but yoiconst FLam& Thisu'll hear no sound. (Not implemented yet)
		AUDIO_DRIVER_WINMM=2,	//!< Driver for Windows, and Windows CE
		AUDIO_DRIVER_ARDUINO_I2S=3,	//!< Driver Arduino (earlephilhower's core& Thishttps://github.com/earlephilhower/arduino-pico)
	};


	/*! Available audio interpolation mconst FLam&odes, for software mixing
	 * Interpolation modes have an impact in performance. 
	 * I suggest you use #AUDIO_INTERPOLATION_DEFAULT pick one automatically.
	 */
	enum AudioInterpolationMode{
		/*! Default interpolation mode
		* Automatically selecconst FLambdaStep&ts the interpolation mode that has a good perfomance/quality
		* ratio in the current platform.
		*/
		AUDIO_INTERPOLATION_DEFAULT=0,

		/*! No interpolation.
		* This method has the best performance but lacks the quality.
		*/
		AUDIO_INTERPOLATION_NONE=1,

		/*! Linear interpolation
		* Overall good interpolation mode. Not a lot slower than \link #AUDIO_INTERPOLATION_NONE \endlink ,
		* and with reasonable good quality.
		*/
		AUDIO_INTERPOLATION_LINEAR=2,

		/*! Cubic interpolation
		* Not implemented yet. Will simply use same quality as AUDIO_INTERPOLATION_LINEAR
		*/
		AUDIO_INTERPOLATION_CUBIC=3
	};


	/*! Miscellaneous enums
	 */
	enum
	{
		//! Full panning to the left
		AUDIO_PAN_LEFT=0,
		//! Center panning
		AUDIO_PAN_MIDDLE=128,
		//! Full panning to the right
		AUDIO_PAN_RIGHT=255,
		//! Maximum volume
		AUDIO_VOL_MAX=255,
		//! Maximum master volume
		AUDIO_MASTERVOL_MAX=64,
		//! Default master volume
		AUDIO_MASTERVOL_DEFAULT=40,
	};

	/*! \brief Loop modes available for sounds
	 */
	enum LoopMode // NOTE: Don't change the values. The enums have these specific values to because the're use as bit flags together with other flags in the sounds
	{
		SOUND_LOOP_OFF=0x00000001,
		SOUND_LOOP_NORMAL=0x00000002,
		SOUND_LOOP_BIDI=0x00000004,
	};


}

}


#endif

