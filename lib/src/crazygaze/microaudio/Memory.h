#pragma once

#include <crazygaze/microaudio/Config.h>
#include <crazygaze/microaudio/MemoryTracker.h>

#if CZ_PLATFORM_WIN32
	#include <memory>
#endif

namespace cz::microaudio
{

//
// Do not use anything in this namespace directly
//
namespace detail
{
	template<typename T>
	struct UniquePtrDeleter
	{
		void operator()(T* ptr)
		{
			(*ptr).~T();
#if CZMICROAUDIO_MEMTRACKER_ENABLED
			cz::microaudio::MemoryTracker::free(ptr);
#else
			::free(b);
#endif
		}
	};
}


template<typename T>
using UniquePtr = std::unique_ptr<T, detail::UniquePtrDeleter<T>>;

//
// Do not use anything in this namespace directly
//
namespace detail
{

#if CZMICROAUDIO_MEMTRACKER_ENABLED

template<typename T, typename... Args>
UniquePtr<T> makeUniqueHelper(const char* file, uint32_t line, Args&&... args)
{
	void* ptr = cz::microaudio::MemoryTracker::alloc(sizeof(T), file, line);
	return UniquePtr<T>(new(ptr) T(std::forward<Args>(args)...));
}

#else

template<typename T, typename... Args>
UniquePtr<T> makeUniqueHelper(Args&&... args)
{
	void* ptr = ::malloc(sizeof(T));
	return UniquePtr<T>(new(ptr) T(std::forward<Args>(args)...));
}
#endif
}


#if CZMICROAUDIO_MEMTRACKER_ENABLED
	#define makeUnique(Type, ...) ::cz::microaudio::detail::makeUniqueHelper<Type>(__FILE__, __LINE__, __VA_ARGS__)
#else
	#define makeUnique(Type, ...) ::cz::microaudio::detail::makeUniqueHelper<Type>(__VA_ARGS__)
#endif

}

