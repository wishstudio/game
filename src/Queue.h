#pragma once

/* Simple and fast thread-unsafe queue. */
/* This is necessary because current std::queue in VC++ is very slow. */
#define QUEUE_BLOCK_SIZE 1024
template <typename T>
class Queue
{
public:
	struct QueueChunk
	{
		T data[QUEUE_BLOCK_SIZE];
		QueueChunk *next;
	};

	Queue(): head(new QueueChunk()), tail(head), pushIndex(0), popIndex(0)
	{
		head->next = nullptr;
	}

	~Queue()
	{
		while (tail)
		{
			QueueChunk *next = tail->next;
			delete tail;
			tail = next;
		}
	}

	bool empty()
	{
		return head == tail && pushIndex == popIndex;
	}

	void push(const T &value)
	{
		if (pushIndex == QUEUE_BLOCK_SIZE)
		{
			QueueChunk *next = new QueueChunk();
			next->next = nullptr;
			head->next = next;
			head = next;
			pushIndex = 0;
		}
		head->data[pushIndex++] = value;
	}

	T pop()
	{
		if (popIndex == QUEUE_BLOCK_SIZE)
		{
			QueueChunk *next = tail->next;
			delete tail;
			tail = next;
			popIndex = 0;
		}
		return std::move(tail->data[popIndex++]);
	}

private:
	QueueChunk *head, *tail;
	int pushIndex, popIndex;
};
#undef QUEUE_BLOCK_SIZE
