#pragma once

#include <crazygaze/microaudio/Config.h>
#include <crazygaze/microaudio/MemoryTracker.h>

#if __has_include (<memory>)
	#include <memory>
#else
	#error "Need STL's <memory>"
#endif

namespace cz::microaudio
{

//
// Do not use anything in this namespace directly
//
namespace detail
{
	template<typename T>
	void doDelete(T* ptr)
	{
		(*ptr).~T();
#if CZMICROAUDIO_MEMTRACKER_ENABLED
		cz::microaudio::MemoryTracker::free(ptr);
#else
		::free(ptr);
#endif
	}

	template<typename T>
	struct UniquePtrDeleter
	{
		void operator()(T* ptr)
		{
			doDelete(ptr);
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
} // detail


} // cz::microaudio

#if CZMICROAUDIO_MEMTRACKER_ENABLED
	#define makeUnique(Type, ...) ::cz::microaudio::detail::makeUniqueHelper<Type>(__FILE__, __LINE__, ##__VA_ARGS__)
#else
	#define makeUnique(Type, ...) ::cz::microaudio::detail::makeUniqueHelper<Type>(##__VA_ARGS__)
#endif


#if CZMICROAUDIO_MEMTRACKER_ENABLED
	#define CZMICROAUDIO_ALLOC(size) MemoryTracker::alloc(size, __FILE__, __LINE__)
	#define CZMICROAUDIO_FREE(ptr) MemoryTracker::free(ptr)

	// #TODO : Revise everythere this is used, to check if I should use UniquePtr<T>
	#define CZMICROAUDIO_NEW(CZOBJECTTYPE, ...) makeUnique(CZOBJECTTYPE, ##__VA_ARGS__).release()
	#define CZMICROAUDIO_DELETE(OBJ) ::cz::microaudio::detail::doDelete(OBJ)

#else
	#define CZMICROAUDIO_ALLOC(size) malloc(size)
	#define CZMICROAUDIO_FREE(ptr) free(ptr)

	// #TODO : Revise everywhere this is used, to check if I should use UniquePtr<T>
	#define CZMICROAUDIO_NEW(CZOBJECTTYPE) makeUnique(CZOBJECTTYPE).release()
	#define CZMICROAUDIO_DELETE(OBJ) ::cz::microaudio::detail::doDelete(OBJ)
#endif

