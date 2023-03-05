#include <crazygaze/microaudio/MemoryTracker.h>
#include <crazygaze/microaudio/PlayerPrivateDefs.h>
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
		CZMICROAUDIO_ASSERT(false);
		return nullptr;
	}

	memset(info, 0, sizeof(AllocInfo));
	void* ptr = info+1;

#if CZMICROAUDIO_MEMTRACKER_CHECK_FREE
	info->ptr = ptr;
#endif

#if CZMICROAUDIO_MEMTRACKER_TRACK_LOCATION
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
#if CZMICROAUDIO_MEMTRACKER_CHECK_FREE
	CZMICROAUDIO_ASSERT(info->ptr == ptr);
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

	printMultipleln(F("Size,File,Line"));

	const AllocInfo* info = ms_list.front();
	while(info)
	{
		totalAllocs++;
		totalRequestedBytes += info->size;

		void* ptr = (void*)0;
		const char* file = "";
		uint32_t line = 0;
#if CZMICROAUDIO_MEMTRACKER_CHECK_FREE
		ptr = info->ptr;
#endif
#if CZMICROAUDIO_MEMTRACKER_TRACK_LOCATION
		file = info->file;
		line = info->line;
#endif
		printMultipleln(info->size, ",", file, ",", line);
		info = info->nextLinkedItem();
	}

	CZMICROAUDIO_ASSERT(totalAllocs == (ms_numAllocs - ms_numFrees));
	printMultiple();

	size_t overheadBytes = totalAllocs * sizeof(AllocInfo);
	printMultipleln(F("Total allocs="), totalAllocs);
	printMultipleln(F("Total requested bytes="), totalRequestedBytes);
	printMultipleln(F("Total overhead="), overheadBytes);
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

	CZMICROAUDIO_ASSERT(totalAllocs == (ms_numAllocs - ms_numFrees));

	if (outAppAllocated)
		*outAppAllocated = appAllocated;
	if (outOverhead)
		*outOverhead = totalAllocs * sizeof(AllocInfo);
}

}


