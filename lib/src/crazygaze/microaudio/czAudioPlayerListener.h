//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Interface for getting callbacks for playback related events (playback end, etc)
//

#ifndef _CZ_AUDIOPLAYERLISTENER_H_
#define _CZ_AUDIOPLAYERLISTENER_H_

#include "czAudio.h"

namespace cz
{

namespace audio
{

	/*!
	* \brief Used to notify you of various playback events
	*
	* If you want to get notified of the events this class provides, you should create a class that implements the required methods, and pass it
	* in  \link Core::Create \endlink.
	*/
	class AudioPlayerListener
	{
	public:

		/*!
		* \brief Callback for when an sound finishes playing
		*
		* 
		* \param sndHandle
		*		Handler you got when calling \link cz::AudioPlayer::Play \endlink, or \link cz::AudioPlayer::PlayModule \endlink
		*
		* \note
		* The sound can be a normal sound, or a module.
		* Also you'll only get this called if the sound stops playing by his own. If you stop the sound yourself, this won't be called.
		*/
		virtual void OnPlayFinished(HSOUND sndHandle) = 0;

		/*!
		 * \brief Callback after a portion of sound is mixed
		 * \param ptr Buffer with the sound data. The sound format is czPlayer output format
		 * \param numFrames Size of the buffer in sound frames.
		 * \note This callback is called from the audio thread.
		 *
		 * This callback provides you with an hook to do whatever you want with the final mix before sending it to the sound device.
		 * The buffer passed as parameter is czPlayer real mix buffer, so be careful with whatever you do with the data.
		 */
		virtual void OnBufferMixed(void* ptr, int numFrames)
		{
		};
	};

} // namespace audio
} // namespace cz

#endif
