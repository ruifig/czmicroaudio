//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include <crazygaze/microaudio/Module.h>


namespace cz
{
namespace microaudio
{

Module::Module(::cz::Core *parentObject) : ::cz::Object(parentObject)
{
	m_loop = false;
	m_reachedEnd = false;
}

Module::~Module()
{
}

} // namespace microaudio
} // namespace cz
