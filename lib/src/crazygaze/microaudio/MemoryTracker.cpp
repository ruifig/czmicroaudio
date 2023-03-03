#include <crazygaze/microaudio/MemoryTracker.h>
#include <Arduino.h>

namespace cz::microaudio
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
	AllocInfo* info = reinterpret_cast<AllocInfo*>(malloc(sizeof(AllocInfo) + size));
	if (!info)
	{
		exit(EXIT_FAILURE);
	}

	memset(info, 0, sizeof(AllocInfo));
	void* ptr = info+1;

#if CZ_MEMORYTRACKER_CHECK_FREE
	info->ptr = ptr;
#endif

#if CZ_MEMORYTRACKER_TRACK_LOCATION
	info->file = file ? getFilename(file) : "";
	info->line = line;
#endif
	info->size = size;

	ms_list.pushBack(info);
	ms_numAllocs++;
	return ptr;
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

	AllocInfo* info = reinterpret_cast<AllocInfo*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(AllocInfo));
#if CZ_MEMORYTRACKER_CHECK_FREE
	assert(info->ptr == ptr);
	if (info->ptr != ptr)
	{
		return;
	}
#endif

	ms_list.remove(info);
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

	printMultipleln("Ptr,File,Line,Size");

	const AllocInfo* info = ms_list.front();
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
		info = info->nextLinkedItem();
	}

	assert(totalAllocs == (ms_numAllocs - ms_numFrees));
	printMultiple();

	size_t overheadBytes = totalAllocs * sizeof(AllocInfo);
	printMultipleln("Total allocs=", totalAllocs);
	printMultipleln("Total requested bytes=", totalRequestedBytes);
	printMultipleln("Total overhead=", overheadBytes);
}

void MemoryTracker::calcAllocated(size_t* outAppAllocated, size_t* outOverhead)
{
	size_t appAllocated = 0;
	size_t overhead = 0;
	size_t totalAllocs = 0;

	const AllocInfo* info = ms_list.front();
	while(info)
	{
		totalAllocs++;
		appAllocated += info->size;
		info = info->nextLinkedItem();
	}

	assert(totalAllocs == (ms_numAllocs - ms_numFrees));

	if (outAppAllocated)
		*outAppAllocated = appAllocated;
	if (outOverhead)
		*outOverhead = totalAllocs * sizeof(AllocInfo);
}

}


