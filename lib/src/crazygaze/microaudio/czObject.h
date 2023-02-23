//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
//  
//

#ifndef _CZ_OBJECT_H_
#define _CZ_OBJECT_H_


#include "czPlayer.h"
#include "czCore.h"

namespace cz
{

	class Object
	{
	public:
		void* operator new (size_t size, ::cz::Core *core);
		void* operator new (size_t size, ::cz::Core *core, const char* file, int line);	
		void operator delete(void *ptr);
		void operator delete (void* ptr, ::cz::Core *core);
		// Just to shut up VC++ on debug mode
		void operator delete (void *ptr, ::cz::Core *core, const char *file, int line);

		virtual ~Object();

	protected:

		friend class ::cz::Core;

		Object(::cz::Core *core); // protected to keep the user from creating objects of this type
		::cz::Core* m_core;

	private:

		void* operator new[] (size_t) throw()
		{
			return NULL;
		};
		void operator delete[] (void*)
		{
		};

	};


} // namespace cz

#endif
