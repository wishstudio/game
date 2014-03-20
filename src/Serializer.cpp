#include "stdafx.h"

#include "lz4/lz4.h"

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

int Serializer::getData(void **data)
{
	*data = malloc(len);
	SerializerBlock *block = first;
	char *i = (char *) *data;
	while (block != current)
	{
		memcpy(i, block->data, BLOCK_SIZE);
		block = block->next;
		i += BLOCK_SIZE;
	}
	memcpy(i, block->data, p);
	return len;
}

int Serializer::getCompressedData(void **data)
{
	void *raw;
	getData(&raw);
	*data = malloc(LZ4_compressBound(len) + 4);
	int32_t compressedLen = LZ4_compress((const char *) raw, (char *) *data + 4, len);
	free(raw);
	*(int32_t *) (*data) = len;
	return compressedLen + 4;
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

void Serializer::writeBlock(const char *data, uint32_t len)
{
	uint32_t s = 0;
	this->len += len;
	while (len > 0)
	{
		uint32_t cl = min(BLOCK_SIZE - p, len);
		memcpy(&current->data[p], &data[s], cl);
		p += cl;
		len -= cl;
		if (p == BLOCK_SIZE)
			appendBlock();
	}
}

Serializer &Serializer::operator << (const uint8_t data)
{
	writeChar(data);
	return *this;
}

Serializer &Serializer::operator << (const uint16_t data)
{
	writeBlock((char *) &data, 2);
	return *this;
}

Serializer &Serializer::operator << (const uint32_t data)
{
	writeBlock((char *) &data, 4);
	return *this;
}

Serializer &Serializer::operator << (const uint64_t data)
{
	writeBlock((char *) &data, 8);
	return *this;
}

Serializer &Serializer::operator << (const int8_t data)
{
	writeChar(data);
	return *this;
}

Serializer &Serializer::operator << (const int16_t data)
{
	writeBlock((char *) &data, 2);
	return *this;
}

Serializer &Serializer::operator << (const int32_t data)
{
	writeBlock((char *) &data, 4);
	return *this;
}

Serializer &Serializer::operator << (const int64_t data)
{
	writeBlock((char *) &data, 8);
	return *this;
}
