#pragma once

template <typename T, int ChunkSize = 4096>
class ConcurrentMemoryPool final
{
private:
	/* TODO: Use unions when VC supports it */
	struct ChunkNode final
	{
		ChunkNode() {}
		~ChunkNode() {}

		std::atomic<ChunkNode *> &asPointerAtomic() { return *(std::atomic<ChunkNode *> *)node; }
		T *asObject() { return (T *)(node); }

		char node[sizeof(T)];
	};

	struct Chunk final
	{
		Chunk(): next(nullptr) {}
		~Chunk() {}

		Chunk *next;
		ChunkNode nodes[ChunkSize];
	};

	void _addToFreeList(ChunkNode *node)
	{
		for (;;)
		{
			ChunkNode *head = freeHead.load();
			node->asPointerAtomic() = head;
			if (freeHead.compare_exchange_weak(head, node))
				return;
		}
	}

	void _newChunk()
	{
		std::lock_guard<std::mutex> lock(chunkMutex);

		Chunk *chunk = new Chunk();
		chunk->next = nullptr;
		if (chunkHead == nullptr) /* We're just initializing */
			chunkHead = chunkTail = chunk;
		else
		{
			chunkTail->next = chunk;
			chunkTail = chunk;
		}
		for (int i = 0; i < ChunkSize; i++)
			_addToFreeList(&chunk->nodes[i]);
	}

	Chunk *chunkHead, *chunkTail;
	std::mutex chunkMutex;

	std::atomic<ChunkNode *> freeHead;

public:
	ConcurrentMemoryPool(): chunkHead(nullptr), chunkTail(nullptr), freeHead(nullptr)
	{
		_newChunk();
	}

	~ConcurrentMemoryPool()
	{
		while (chunkHead)
		{
			Chunk *chunk = chunkHead->next;
			delete chunkHead;
			chunkHead = chunk;
		}
	}

	T *allocateObject()
	{
		for (;;)
		{
			ChunkNode *head = freeHead.load();
			if (head == nullptr)
			{
				/* No free nodes */
				_newChunk();
			}
			else
			{
				ChunkNode *next = head->asPointerAtomic().load();
				/* Note 'next' may contains corrupted value since the atomic part is interleaved with real data
				 * But anyway the compare and exchange operation will definitely fail so 'next' wouldn't be used
				 */
				if (freeHead.compare_exchange_weak(head, next))
					return head->asObject();
			}
		}
	}

	void deleteObject(T *object)
	{
		ChunkNode *node = (ChunkNode *) object;
		node->asPointerAtomic() = nullptr;
		_addToFreeList(node);
	}
};
