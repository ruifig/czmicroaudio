//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/Module.h>

namespace cz::microaudio
{

Module::Module()
{
	m_loop = false;
	m_reachedEnd = false;
}

Module::~Module()
{
}

} // namespace cz::microaudio

