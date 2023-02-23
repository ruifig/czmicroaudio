//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czModule.h"


namespace cz
{
namespace audio
{

Module::Module(::cz::Core *parentObject) : ::cz::Object(parentObject)
{
	m_loop = false;
	m_reachedEnd = false;
}

Module::~Module()
{
}

} // namespace audio
} // namespace cz