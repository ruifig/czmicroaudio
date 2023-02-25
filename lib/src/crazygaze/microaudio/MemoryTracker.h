#pragma once

#include <stdint.h>
#include <crazygaze/microaudio/Config.h>

namespace cz
{

#define CZ_MEMORYTRACKER_TRACK_LOCATION 1
#define CZ_MEMORYTRACKER_CHECK_FREE 1

struct AllocationInfo
{
#if CZ_MEMORYTRACKER_CHECK_FREE
	 // The actual ptr the application uses.
	 // This is only needed to validate if a free was passed a valid pointer.
	void* ptr;
#endif

#if CZ_MEMORYTRACKER_TRACK_LOCATION
	const char* file;
	uint32_t line;
#endif
	size_t size; // size in bytes

	AllocationInfo* previous;
	AllocationInfo* next;
};

class MemoryTracker
{
public:
	
	static void* alloc(size_t size);
	static void* alloc(size_t size, const char* file, uint32_t line);
	static void free(void* ptr);
	static void log();

private:
	inline static AllocationInfo* ms_last = nullptr;
	inline static size_t ms_numAllocs = 0;
	inline static size_t ms_numFrees = 0;
};

}

