#pragma once

template <typename T>
static inline T hash(T value);

template <>
static inline u32 hash(u32 i)
{
	return i;
}

template <>
static inline s32 hash(s32 i)
{
	return i;
}

#define _HASH_END 0xFFFFFFFFU
#define _HASH_STEP 0x3U
template <typename KeyType1, typename KeyType2, typename KeyType3, typename ValueType>
class Hash
{
public:

	class iterator
	{
	public:
		bool operator == (const iterator &another)
		{
			return hash == another.hash && i == another.i;
		}

		bool operator != (const iterator &another)
		{
			return hash != another.hash || i != another.i;
		}

		iterator operator ++ ()
		{
			for (;;)
			{
				i++;
				if (i == hash->capacity)
				{
					i = _HASH_END;
					return *this;
				}
				if (hash->nodes[i].exists)
					return *this;
			}
		}

		ValueType operator * ()
		{
			return hash->nodes[i].value;
		}

	private:
		iterator(Hash *_hash, u32 _i): hash(_hash), i(_i) {}

		Hash *hash;
		u32 i;
		friend class Hash;
	};

	Hash()
	{
		capacity = 4;
		currentSize = 0;
		nodes = new HashNode[capacity];
		memset(nodes, 0, sizeof(HashNode) * capacity);
	}

	~Hash()
	{
		delete nodes;
	}

	u32 size() const
	{
		return currentSize;
	}

	void insert(KeyType1 key1, KeyType2 key2, KeyType3 key3, ValueType value)
	{
		currentSize++;
		if (currentSize * 2 > capacity)
			rehash();
		u32 h = combinehash(key1, key2, key3);
		while (nodes[h].exists)
		{
			if (nodes[h].key1 == key1 && nodes[h].key2 == key2 && nodes[h].key3 == key3)
				return;
			h += _HASH_STEP;
			if (h >= capacity)
				h -= capacity;
		}
		nodes[h].key1 = key1;
		nodes[h].key2 = key2;
		nodes[h].key3 = key3;
		nodes[h].value = value;
		nodes[h].exists = true;
	}

	iterator find(KeyType1 key1, KeyType2 key2, KeyType3 key3)
	{
		u32 h = combinehash(key1, key2, key3);
		while (nodes[h].exists)
		{
			if (nodes[h].key1 == key1 && nodes[h].key2 == key2 && nodes[h].key3 == key3)
				return iterator(this, h);
			h += _HASH_STEP;
			if (h >= capacity)
				h -= capacity;
		}
		return end();
	}

	iterator begin()
	{
		for (u32 i = 0; i < capacity; i++)
			if (nodes[i].exists)
				return iterator(this, i);
		return end();
	}

	iterator end()
	{
		return iterator(this, _HASH_END);
	}

private:
	u32 combinehash(KeyType1 key1, KeyType2 key2, KeyType3 key3)
	{
		return (hash(key1) ^ hash(key2) ^ hash(key3)) % capacity;
	}

	void rehash()
	{
		u32 old_capacity = capacity;
		capacity *= 2;
		HashNode *old = nodes;
		nodes = new HashNode[capacity];
		memset(nodes, 0, sizeof(HashNode) * capacity);
		currentSize = 0;
		for (u32 i = 0; i < old_capacity; i++)
			if (old[i].exists)
				insert(old[i].key1, old[i].key2, old[i].key3, old[i].value);
		delete old;
	}

	struct HashNode
	{
		KeyType1 key1;
		KeyType2 key2;
		KeyType3 key3;
		ValueType value;
		bool exists;
	};
	HashNode *nodes;
	u32 currentSize, capacity;
};
