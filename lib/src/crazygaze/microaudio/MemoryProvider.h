//
// CrazyGaze (http://www.crazygaze.com)
// Author : Rui Figueira
// Email  : rui@crazygaze.com
// 
// --------------------------------------------------------------
// Interface for memory allocation
//

#ifndef _CZ_MEMORY_PROVIDER_H_
#define _CZ_MEMORY_PROVIDER_H_

namespace cz
{

/*!
 * \brief Provide memory allocation for the entire library
 * You can use an implementation of this class to provide your own memory allocation
 *
 * \sa \link ::cz::Core::Create \endlink
 *
 */
class MemoryProvider
{
public:
	//! Called to allocate memory. Should behave like standard C function "malloc".
	virtual void *AllocMem(size_t size) = 0;
	//! Called to free memory. Should behave like standard C function "free".
	virtual void FreeMem(void *ptr) = 0;
};

}


#endif
