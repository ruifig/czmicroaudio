#include <crazygaze/microaudio/MemoryTracker.h>
#include <Arduino.h>

namespace cz
{

const char* getFilename(const char* file)
{
	const char* a = strrchr(file, '\\');
	const char* b = strrchr(file, '/');
	const char* c = a > b ? a : b;
	return c ? c+1 : file;
}

void* MemoryTracker::alloc(size_t size, const char* file, uint32_t line)
{
	AllocationInfo* info = reinterpret_cast<AllocationInfo*>(malloc(sizeof(AllocationInfo) + size));
	if (!info)
	{
		exit(EXIT_FAILURE);
	}

#if CZ_MEMORYTRACKER_CHECK_FREE
	info->ptr = info+1;
#endif

#if CZ_MEMORYTRACKER_TRACK_LOCATION
	info->file = file ? getFilename(file) : "";
	info->line = line;
#endif
	info->size = size;

	info->previous = ms_last;
	info->next = nullptr;
	if (ms_last)
	{
		ms_last->next = info;
	}
	ms_last = info;

	ms_numAllocs++;
	return info->ptr;
}

void* MemoryTracker::alloc(size_t size)
{
	return alloc(size, nullptr, 0);
}

void MemoryTracker::free(void* ptr)
{
	if (ptr==nullptr)
	{
		return;
	}

	AllocationInfo* info = reinterpret_cast<AllocationInfo*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(AllocationInfo));
#if CZ_MEMORYTRACKER_CHECK_FREE
	assert(info->ptr == ptr);
	if (info->ptr != ptr)
	{
		return;
	}
#endif

	if (info == ms_last)
	{
		ms_last = info->previous;
	}

	if (info->previous)
	{
		info->previous->next = info->next;
	}
	if (info->next)
	{
		info->next->previous = info->previous;
	}

	::free(info);

	ms_numFrees++;
}

template<class... T>
void printMultiple(T&&... args)
{
	(Serial.print(args),...);
}

template<class... T>
void printMultipleln(T&&... args)
{
	(Serial.print(args),...);
	Serial.println("");
}

void MemoryTracker::log()
{
	size_t totalAllocs = 0;
	size_t totalRequestedBytes = 0;

	// Find the first element
	const AllocationInfo* info = ms_last;
	while(info && info->previous)
	{
		info = info->previous;
	}

	printMultipleln("Ptr,File,Line,Size");

	while(info)
	{
		totalAllocs++;
		totalRequestedBytes += info->size;

		void* ptr = (void*)0;
		const char* file = "";
		uint32_t line = 0;
#if CZ_MEMORYTRACKER_CHECK_FREE
		ptr = info->ptr;
#endif
#if CZ_MEMORYTRACKER_TRACK_LOCATION
		file = info->file;
		line = info->line;
#endif

	printMultipleln(ptr, ",", file, ",", line, ",", info->size);
	#if 0
	Serial.print(ptr); Serial.print(",");
	Serial.print(file); Serial.print(",");
	Serial.print(line); Serial.print(",");
	Serial.println(size);
	#endif

		info = info->next;
	}

	assert(totalAllocs == (ms_numAllocs - ms_numFrees));
	printMultiple();

	size_t overheadBytes = totalAllocs * sizeof(AllocationInfo);
	printMultipleln("Total allocs=", totalAllocs);
	printMultipleln("Total requested bytes=", totalRequestedBytes);
	printMultipleln("Total overhead=", overheadBytes);
}

}