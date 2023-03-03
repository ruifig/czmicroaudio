#include <crazygaze/microaudio/LinkedList.h>
#include <crazygaze/mut/mut.h>

namespace
{

template<typename T>
class TestableDoublyLinked : cz::microaudio::DoublyLinked<T>
{
	public:
	using cz::microaudio::DoublyLinked<T>::m_previous;
	using cz::microaudio::DoublyLinked<T>::m_next;
};

struct Foo : public TestableDoublyLinked<Foo>
{
	explicit Foo(int n) : n(n) {}
	int n;
};

struct LinkedListTestHarness
{
	cz::microaudio::DoublyLinkedList<Foo> list;
	Foo foos[6] = {
		Foo(0),
		Foo(1),
		Foo(2),
		Foo(3),
		Foo(4),
		Foo(5)
	};

	void checkAllUnlinked()
	{
		for(auto&& f : foos)
		{
			CHECK(f.m_previous == nullptr);
			CHECK(f.m_next == nullptr);
		}
	}

	// Checks if the given foo (index idx) is linked to any other foos
	// previousIdx/nextIdx can be specified as -1 which means no link
	void checkLinks(int idx, int previousIdx, int nextIdx)
	{
		Foo& f = foos[idx];
		if (previousIdx == -1)
		{
			CHECK(f.m_previous == nullptr);
		}
		else
		{
			CHECK(f.m_previous && f.m_previous->n == previousIdx);
		}

		if (nextIdx == -1)
		{
			CHECK(f.m_next == nullptr);
		}
		else
		{
			CHECK(f.m_next && f.m_next->n == nextIdx);
		}
	}

};

}

TEST_CASE("LinkedList-empty()", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;
	CHECK(harness.list.empty() == true);
	harness.list.pushBack(&harness.foos[0]);
	CHECK(harness.list.empty() == false);
};

TEST_CASE("LinkedList-pushBack()", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;

	SECTION("The first item inserted should populate m_first and m_last")
	{
		harness.list.pushBack(&harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[0]);
		CHECK(harness.list.front() == &harness.foos[0]);
		harness.checkLinks(0, -1, -1); // No links should be create because it's the only item
	}

	SECTION("New items should be inserted at the end")
	{
		harness.list.pushBack(&harness.foos[0]);
		harness.list.pushBack(&harness.foos[1]);

		CHECK(harness.list.front() == &harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[1]);

		harness.checkLinks(0, -1, 1);
		harness.checkLinks(1, 0, -1);

		harness.list.pushBack(&harness.foos[2]);
		CHECK(harness.list.front() == &harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[2]);

		harness.checkLinks(0, -1, 1);
		harness.checkLinks(1, 0, 2);
		harness.checkLinks(2, 1, -1);
	}
}

TEST_CASE("LinkedList-pushFront()", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;

	SECTION("The first item inserted should populate m_first and m_last")
	{
		harness.list.pushFront(&harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[0]);
		CHECK(harness.list.front() == &harness.foos[0]);
		harness.checkLinks(0, -1, -1); // No links should be create because it's the only item
	}

	SECTION("New items should be inserted at the beginning")
	{
		harness.list.pushFront(&harness.foos[2]);
		harness.list.pushFront(&harness.foos[1]);

		CHECK(harness.list.front() == &harness.foos[1]);
		CHECK(harness.list.back() == &harness.foos[2]);

		harness.checkLinks(1, -1, 2);
		harness.checkLinks(2, 1, -1);

		harness.list.pushFront(&harness.foos[0]);

		harness.checkLinks(0, -1, 1);
		harness.checkLinks(1, 0, 2);
		harness.checkLinks(2, 1, -1);
	}
}


TEST_CASE("LinkedList-popBack()", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;

	SECTION("When empty is a nop")
	{
		harness.list.popBack();
	}

	SECTION("When it's the last item, it should reset m_first and m_last")
	{
		harness.list.pushBack(&harness.foos[0]);
		harness.list.popBack();
		harness.checkLinks(0, -1, -1);
		CHECK(harness.list.back() == nullptr);
		CHECK(harness.list.front() == nullptr);
	}

	SECTION("Should remove only 1 item at a time")
	{
		harness.list.pushBack(&harness.foos[0]);
		harness.list.pushBack(&harness.foos[1]);
		harness.list.pushBack(&harness.foos[2]);

		CHECK(harness.list.front() == &harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[2]);
		harness.checkLinks(0, -1, 1);
		harness.checkLinks(1, 0, 2);
		harness.checkLinks(2, 1, -1);

		harness.list.popBack(); // pop f2
		CHECK(harness.list.front() == &harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[1]);
		harness.checkLinks(0, -1, 1);
		harness.checkLinks(1, 0, -1);
		harness.checkLinks(2, -1, -1);

		harness.list.popBack(); // pop f1
		CHECK(harness.list.front() == &harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[0]);
		harness.checkLinks(0, -1, -1);
		harness.checkLinks(1, -1, -1);

		harness.list.popBack(); // pop f0
		CHECK(harness.list.front() == nullptr);
		CHECK(harness.list.back() == nullptr);
		harness.checkLinks(0, -1, -1);
	}
}

TEST_CASE("LinkedList-popFront()", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;

	SECTION("When empty is a nop")
	{
		harness.list.popFront();
	}

	SECTION("When it's the last item, it should reset m_first and m_last")
	{
		harness.list.pushBack(&harness.foos[0]);
		harness.list.popFront();
		harness.checkLinks(0, -1, -1);
		CHECK(harness.list.back() == nullptr);
		CHECK(harness.list.front() == nullptr);
	}

	SECTION("Should remove only 1 item at a time")
	{
		harness.list.pushBack(&harness.foos[0]);
		harness.list.pushBack(&harness.foos[1]);
		harness.list.pushBack(&harness.foos[2]);

		CHECK(harness.list.front() == &harness.foos[0]);
		CHECK(harness.list.back() == &harness.foos[2]);
		harness.checkLinks(0, -1, 1);
		harness.checkLinks(1, 0, 2);
		harness.checkLinks(2, 1, -1);

		harness.list.popFront(); // pop f0
		CHECK(harness.list.front() == &harness.foos[1]);
		CHECK(harness.list.back() == &harness.foos[2]);
		harness.checkLinks(0, -1, -1);
		harness.checkLinks(1, -1, 2);
		harness.checkLinks(2, 1, -1);

		harness.list.popFront(); // pop f1
		CHECK(harness.list.front() == &harness.foos[2]);
		CHECK(harness.list.back() == &harness.foos[2]);
		harness.checkLinks(1, -1, -1);
		harness.checkLinks(2, -1, -1);

		harness.list.popFront(); // pop f2
		CHECK(harness.list.front() == nullptr);
		CHECK(harness.list.back() == nullptr);
		harness.checkLinks(2, -1, -1);
	}
}

TEST_CASE("LinkedList-clear()", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;
	harness.list.pushBack(&harness.foos[0]);

	CHECK(harness.list.back() == &harness.foos[0]);
	CHECK(harness.list.front() == &harness.foos[0]);

	harness.list.pushBack(&harness.foos[1]);
	harness.list.pushBack(&harness.foos[2]);

	harness.list.clear();

	CHECK(harness.list.front() == nullptr);
	CHECK(harness.list.back() == nullptr);
	harness.checkAllUnlinked();
}

TEST_CASE("LinkedList tests", "[czmicroaudio][linkedlist]")
{
	cz::microaudio::DoublyLinkedList<Foo> list;

	Foo f0(0), f1(1), f2(2), f3(3), f4(4), f5(5);
}

TEST_CASE("LinkedList-insertAfter", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;

	harness.list.insertAfter(&harness.foos[0], nullptr);
	CHECK(harness.list.front() == &harness.foos[0]);
	CHECK(harness.list.back() == &harness.foos[0]);

	harness.list.insertAfter(&harness.foos[2], &harness.foos[0]);
	CHECK(harness.list.front() == &harness.foos[0]);
	CHECK(harness.list.back() == &harness.foos[2]);
	harness.checkLinks(0, -1, 2);
	harness.checkLinks(2, 0, -1);

	// Insert in the middle
	harness.list.insertAfter(&harness.foos[1], &harness.foos[0]);
	CHECK(harness.list.front() == &harness.foos[0]);
	CHECK(harness.list.back() == &harness.foos[2]);
	harness.checkLinks(0, -1, 1);
	harness.checkLinks(1, 0, 2);
	harness.checkLinks(2, 1, -1);
}

TEST_CASE("LinkedList-insertBefore", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;

	harness.list.insertBefore(&harness.foos[2], nullptr);
	CHECK(harness.list.front() == &harness.foos[2]);
	CHECK(harness.list.back() == &harness.foos[2]);

	harness.list.insertBefore(&harness.foos[0], &harness.foos[2]);
	CHECK(harness.list.front() == &harness.foos[0]);
	CHECK(harness.list.back() == &harness.foos[2]);
	harness.checkLinks(0, -1, 2);
	harness.checkLinks(2, 0, -1);

	// Insert in the middle
	harness.list.insertBefore(&harness.foos[1], &harness.foos[2]);
	CHECK(harness.list.front() == &harness.foos[0]);
	CHECK(harness.list.back() == &harness.foos[2]);
	harness.checkLinks(0, -1, 1);
	harness.checkLinks(1, 0, 2);
	harness.checkLinks(2, 1, -1);
}

TEST_CASE("LinkedList-remove", "[czmicroaudio][linkedlist]")
{
	LinkedListTestHarness harness;
	harness.list.pushBack(&harness.foos[0]);
	harness.list.pushBack(&harness.foos[1]);
	harness.list.pushBack(&harness.foos[2]);
	harness.list.pushBack(&harness.foos[3]);
	harness.list.pushBack(&harness.foos[4]);

	// Remove from one side
	harness.list.remove(&harness.foos[0]);
	harness.checkLinks(0, -1, -1);
	harness.checkLinks(1, -1, 2);
	CHECK(harness.list.front() == &harness.foos[1]);
	CHECK(harness.list.back() == &harness.foos[4]);

	// Remove from the other side
	harness.list.remove(&harness.foos[4]);
	harness.checkLinks(4, -1, -1);
	harness.checkLinks(3, 2, -1);
	CHECK(harness.list.front() == &harness.foos[1]);
	CHECK(harness.list.back() == &harness.foos[3]);

	// Remove from the middle
	harness.list.remove(&harness.foos[2]);
	harness.checkLinks(2, -1, -1);
	harness.checkLinks(1, -1, 3);
	harness.checkLinks(3, 1, -1);
	CHECK(harness.list.front() == &harness.foos[1]);
	CHECK(harness.list.back() == &harness.foos[3]);
}


