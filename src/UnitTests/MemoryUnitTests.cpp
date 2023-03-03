#include <crazygaze/microaudio/Memory.h>
#include <crazygaze/mut/mut.h>

using namespace cz::microaudio;

namespace
{

int g_created = 0;
int g_destroyed = 0;

struct Foo
{
	Foo() : a(0xFFFFFFFF)
	{
		g_created++;
	}

	explicit Foo(uint32_t a) : a(a)
	{
		g_created++;
	}

	~Foo()
	{
		g_destroyed++;
	}

	uint32_t a = 0xBAADF00D;
};


struct UniquePtrTestHarness
{
	UniquePtrTestHarness()
	{
		g_created = 0;
		g_destroyed = 0;
	}

	size_t calcAppAllocated() const
	{
		size_t ret;
		MemoryTracker::calcAllocated(&ret, nullptr);
		return ret;
	}

	size_t calcOverhead() const
	{
		size_t ret;
		MemoryTracker::calcAllocated(nullptr, &ret);
		return ret;
	}

	// Putting this here so we can ignore the checks if we are not using the memory tracker
	void checkAllocated(size_t size)
	{
#if CZMICROAUDIO_MEMTRACKER_ENABLED
		CHECK(calcAppAllocated() == size);
#endif
	}

};

}

TEST_CASE("UniquePtr", "[czmicroaudio][uniqueptr]")
{
	UniquePtrTestHarness harness;

	auto p1 = makeUnique(Foo);
	CHECK(p1->a == 0xFFFFFFFF);
	CHECK(g_created == 1);

	harness.checkAllocated(sizeof(Foo));

	p1 = nullptr;
	CHECK(g_destroyed == 1);
	harness.checkAllocated(0);

}
