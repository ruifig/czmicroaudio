#pragma once

#include <crazygaze/microaudio/Config.h>
#include <crazygaze/microaudio/LinkedList.h>

#ifndef CZMICROAUDIO_MEMTRACKER_TRACK_LOCATION
	#if CZMICROAUDIO_DEBUG
		#define CZMICROAUDIO_MEMTRACKER_TRACK_LOCATION 1
	#else
		#define CZMICROAUDIO_MEMTRACKER_TRACK_LOCATION 0
	#endif
#endif

#ifndef CZMICROAUDIO_MEMTRACKER_CHECK_FREE
	#if CZMICROAUDIO_DEBUG
		#define CZMICROAUDIO_MEMTRACKER_CHECK_FREE 1
	#else
		#define CZMICROAUDIO_MEMTRACKER_CHECK_FREE 0
	#endif
#endif

namespace cz::microaudio
{

class MemoryTracker
{
public:

	struct AllocInfo : DoublyLinked<AllocInfo>
	{
	#if CZMICROAUDIO_MEMTRACKER_CHECK_FREE
		 // The actual ptr the application uses.
		 // This is only needed to validate if a free was passed a valid pointer.
		void* ptr;
	#endif

	#if CZMICROAUDIO_MEMTRACKER_TRACK_LOCATION
		const char* file;
		uint32_t line;
	#endif
		size_t size; // size in bytes
	};
	
	static void* alloc(size_t size);
	static void* alloc(size_t size, const char* file, uint32_t line);
	static void free(void* ptr);
	static void log();

	/**
	 * Returns the total number of allocations that occurred since the start of the application
	 */
	static size_t getNumAllocs() { return ms_numAllocs; }

	/**
	 * Returns the total number of deallocations that occurred since the start of the application
	 */
	static size_t getNumDeallocs() { return ms_numFrees; }

	/**
	 * Returns the current number of allocations
	 */
	static size_t getNumCurrentAllocs() { return ms_numAllocs - ms_numFrees; }


	/**
	 * Calculate how many bytes are currently allocated
	 *
	 */
	static void calcAllocated(size_t* outAppAllocated, size_t* outOverhead);

private:
	inline static DoublyLinkedList<AllocInfo> ms_list;
	inline static size_t ms_numAllocs = 0;
	inline static size_t ms_numFrees = 0;
};

}

