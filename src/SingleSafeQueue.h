#pragma once

#include <atomic>
#include <thread>
#include <mutex>

/* Single one producer one consumer queue */
template<typename T>
class SingleSafeQueue
{
public:
	SingleSafeQueue(): head(new Node(T(), nullptr)), tail(head) {}

	void push(const T value)
	{
		Node *node = new Node(value, nullptr);
		tail->next = node;
		tail = node;
	}

	T pop()
	{
		if (head == tail)
			return nullptr;
		volatile Node *old = head;
		head = head->next;
		delete old;
		return head->value;
	}

private:
	struct Node
	{
		const T value;
		Node * volatile next;

		Node(const T _value, Node *_next): value(_value), next(_next) {}
	};
	volatile Node *head, *tail;
};
