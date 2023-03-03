//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#pragma once

namespace cz::microaudio
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


