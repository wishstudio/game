#pragma once

#include "ConcurrentMemoryPool.h"

template <typename T>
class ConcurrentStack final
{
private:
	struct Node
	{
		T value;
		std::atomic<Node *> next;
	};
	std::atomic<Node *> head;
	ConcurrentMemoryPool<Node> memoryPool;

public:
	ConcurrentStack(): head(nullptr) {}
	~ConcurrentStack()
	{
		Node *h = head.load();
		while (h != nullptr)
		{
			Node *next = h->next.load();
			memoryPool.deleteObject(h);
			h = next;
		}
	}

	void push(const T &value)
	{
		Node *node = memoryPool.allocateObject();
		new (&node->value) T(value);
		for (;;)
		{
			Node *h = head.load();
			node->next = h;
			if (head.compare_exchange_weak(h, node))
				break;
		}
	}

	bool empty()
	{
		return head.load() == nullptr;
	}

	bool try_pop(T &value)
	{
		for (;;)
		{
			Node *h = head.load();
			if (h == nullptr)
				return false;
			Node *next = h->next.load();
			if (head.compare_exchange_weak(h, next))
			{
				value = std::move(h->value);
				h->value.~T();
				memoryPool.deleteObject(h);
				return true;
			}
		}
	}
};
