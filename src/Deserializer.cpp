#include "stdafx.h"

#include "lz4/lz4.h"

#include "Serialization.h"

Deserializer::Deserializer(const void *data, u32 len, bool isCompressed)
{
	if (isCompressed)
	{
		u32 rawLen = *(u32 *) data;
		char *d = (char *) malloc(rawLen);
		u32 l = LZ4_decompress_safe((const char *) data + 4, d, len - 4, rawLen);
		/* assert(rawLen == l); */
		this->data = d;
		this->len = rawLen;
		this->p = 0;
		this->ownData = true;
	}
	else
	{
		this->data = (const char *) data;
		this->len = len;
		this->p = 0;
		this->ownData = false;
	}
}

Deserializer::~Deserializer()
{
	if (ownData)
		free((void *) data);
}

char Deserializer::readChar()
{
	return data[p++];
}

void Deserializer::readBlock(char *data, u32 len)
{
	memcpy(data, this->data + p, len);
	p += len;
}

Deserializer &Deserializer::operator >> (u8 &data)
{
	data = readChar();
	return *this;
}

Deserializer &Deserializer::operator >> (u16 &data)
{
	readBlock((char *) &data, 2);
	return *this;
}

Deserializer &Deserializer::operator >> (u32 &data)
{
	readBlock((char *) &data, 4);
	return *this;
}

Deserializer &Deserializer::operator >> (u64 &data)
{
	readBlock((char *) &data, 8);
	return *this;
}

Deserializer &Deserializer::operator >> (s8 &data)
{
	data = readChar();
	return *this;
}

Deserializer &Deserializer::operator >> (s16 &data)
{
	readBlock((char *) &data, 2);
	return *this;
}

Deserializer &Deserializer::operator >> (s32 &data)
{
	readBlock((char *) &data, 4);
	return *this;
}

Deserializer &Deserializer::operator >> (s64 &data)
{
	readBlock((char *) &data, 8);
	return *this;
}
