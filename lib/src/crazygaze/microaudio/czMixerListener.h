//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#ifndef _CZMIXER_LISTENER_H
#define _CZMIXER_LISTENER_H

namespace cz
{
namespace audio
{

	class ChannelMixingListener
	{
	public:
		//! This is called from inside the mixer
		/*!
		 * \return
		 *		If it returns true, the mixer should continue processing the channel
		 *		If it returns false, the mixer will stop the channel
		 */
		virtual bool ChannelMix(int mixpos, int numframes) = 0;
	};

}
}

#endif

