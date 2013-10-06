#include "stdafx.h"

#include "Serialization.h"

static const int BLOCK_SIZE = 4096;

struct SerializerBlock
{
	char data[BLOCK_SIZE];
	SerializerBlock *next;
};

Serializer::Serializer()
{
	first = new SerializerBlock();
	first->next = nullptr;
	current = first;
	p = 0;
	len = 0;
}

Serializer::~Serializer()
{
	while (first != nullptr)
	{
		current = first->next;
		delete first;
		first = current;
	}
}

void *Serializer::getData()
{
	char *data = (char *) malloc(len);
	SerializerBlock *block = first;
	char *i = data;
	while (block != current)
	{
		memcpy(i, block->data, BLOCK_SIZE);
		block = block->next;
		i += BLOCK_SIZE;
	}
	memcpy(i, block->data, p);
	return data;
}

void Serializer::appendBlock()
{
	SerializerBlock *block = new SerializerBlock();
	block->next = nullptr;
	current->next = block;
	current = block;
	p = 0;
}

void Serializer::writeChar(const char ch)
{
	if (p == BLOCK_SIZE)
		appendBlock();
	current->data[p++] = ch;
	len++;
}

void Serializer::writeBlock(const char *data, u32 len)
{
	u32 s = 0;
	this->len += len;
	while (len > 0)
	{
		u32 cl = min(BLOCK_SIZE - p, len);
		memcpy(&current->data[p], &data[s], cl);
		p += cl;
		len -= cl;
		if (p == BLOCK_SIZE)
			appendBlock();
	}
}

Serializer &Serializer::operator << (const u8 data)
{
	writeChar(data);
	return *this;
}

Serializer &Serializer::operator << (const u16 data)
{
	writeBlock((char *) &data, 2);
	return *this;
}

Serializer &Serializer::operator << (const u32 data)
{
	writeBlock((char *) &data, 4);
	return *this;
}

Serializer &Serializer::operator << (const u64 data)
{
	writeBlock((char *) &data, 8);
	return *this;
}

Serializer &Serializer::operator << (const s8 data)
{
	writeChar(data);
	return *this;
}

Serializer &Serializer::operator << (const s16 data)
{
	writeBlock((char *) &data, 2);
	return *this;
}

Serializer &Serializer::operator << (const s32 data)
{
	writeBlock((char *) &data, 4);
	return *this;
}

Serializer &Serializer::operator << (const s64 data)
{
	writeBlock((char *) &data, 8);
	return *this;
}
