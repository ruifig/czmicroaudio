#pragma once

#include <assert.h>

namespace cz::microaudio
{

template<typename T>
class DoublyLinkedList;

template<typename T>
class DoublyLinked
{
public:
	T* nextLinkedItem() { return static_cast<T*>(m_next); }
	const T* nextLinkedItem() const { return static_cast<T*>(m_next); }
	T* previousLinkedItem() { return static_cast<T*>(m_previous); }
	const T* previousLinkedItem() const { return static_cast<T*>(m_previous); }

protected:
	friend DoublyLinkedList<T>;
	T* m_previous = nullptr;
	T* m_next = nullptr;
};

template<typename T>
class DoublyLinkedList
{
public:
	using Item = T;

	bool empty() const
	{
		return m_first == nullptr ? true : false;
	}

	void pushBack(Item* item)
	{
		insertAfter(item, m_last);
	}

	void pushFront(Item* item)
	{
		insertBefore(item, m_first);
	}

	void popBack()
	{
		if (m_last)
		{
			remove(m_last);
		}
	}

	void popFront()
	{
		if (m_first)
		{
			remove(m_first);
		}
	}

	Item* front() { return m_first; }
	const T* front() const { return m_first; }

	Item* back() { return m_last; }
	const T* back() const { return m_last; }

	void clear()
	{
		Item* item = m_first;
		
		while(item)
		{
			Item* tmp = item->m_next;
			item->m_previous = nullptr;
			item->m_next = nullptr;
			item = tmp;
		}

		m_first = m_last = nullptr;
	}

	/**
	 * Inserts an item after another specified item
	 *
	 * \param item
	 *		Item to insert
	 * \param where
	 *		Item after which to insert.
	 *
	 * If the list is empty, "where" can be specified as nullptr. This is intentional, so that
	 * a "list.insertAfter(item, list.back())" or "list.insertAfter(item, list.front())" works even if the
	 * list is empty
	 *
	 */
	void insertAfter(Item* item, Item* where)
	{
		assert(item->m_previous == nullptr && item->m_next == nullptr);

		//
		// [ Where ]   [ B ]
		//           ^
		//         Item
		//
		// Inserting between Where and B.
		// Requires updating Where->next and B->previous
		if (where)
		{
			Item* b = where->m_next;

			// updated inserted item
			item->m_next = b;
			item->m_previous = where;
			// update B
			if (b)
				b->m_previous = item;
			// update Where
			where->m_next = item;

			if (where == m_last)
				m_last = item;
		}
		else
		{
			assert(m_first==nullptr && m_last==nullptr);
			m_first = m_last = item;
		}
	}

	void insertBefore(Item* item, Item* where)
	{
		assert(item->m_previous == nullptr && item->m_next == nullptr);

		//
		// [B]   [ Where ]
		//     ^
		//   Item
		//
		// Inserting between B and Where.
		// Requires updating B->next and Where->previous

		if (where)
		{
			Item* b = where->m_previous;

			// update inserted item
			item->m_next = where;
			item->m_previous = b;
			// update B
			if (b)
				b->m_next = item;
			// update Where
			where->m_previous = item;

			if (where == m_first)
				m_first = item;
		}
		else
		{
			assert(m_first==nullptr && m_last==nullptr);
			m_first = m_last = item;
		}
	}

	void remove(Item* item)
	{
		if (item == m_first)
		{
			m_first = item->m_next;
		}

		if (item == m_last)
		{
			m_last = item->m_previous;
		}

		if (item->m_previous)
		{
			item->m_previous->m_next = item->m_next;
		}

		if (item->m_next)
		{
			item->m_next->m_previous = item->m_previous;
		}

		item->m_previous = nullptr;
		item->m_next = nullptr;
	}

private:
	T* m_first = nullptr;
	T* m_last = nullptr;
};



}
