//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// 
//

#include "czPlayerPrivateDefs.h"
#include "czObject.h"
#include "czStandardC.h"

namespace cz
{


Object::Object(::cz::Core* core) : m_core(core)
{
}

Object::~Object()
{
}


void* Object::operator new (size_t size, ::cz::Core *core)
{
	return core->AllocMem(size);
}


void Object::operator delete (void *ptr)
{
	Core *core = ((Object*)ptr)->m_core;
	core->FreeMem(ptr);
}

void * Object::operator new (size_t size, ::cz::Core *core, const char* file, int line)
{
	return core->AllocMem(size, file, line);
}

void Object::operator delete (void* ptr, ::cz::Core* CZUNUSED(core))
{
	Object::operator delete(ptr);
}

void Object::operator delete (void *ptr, ::cz::Core* CZUNUSED(core), const char* CZUNUSED(file), int CZUNUSED(line))
{
	Object::operator delete(ptr);
}

} // namespace cz