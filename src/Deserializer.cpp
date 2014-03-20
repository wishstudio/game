#include "stdafx.h"

#include "lz4/lz4.h"

#include "Serialization.h"

Deserializer::Deserializer(const void *data, int len, bool isCompressed)
{
	if (isCompressed)
	{
		int32_t rawLen = *(int32_t *) data;
		char *d = (char *) malloc(rawLen);
		int l = LZ4_decompress_safe((const char *) data + 4, d, len - 4, rawLen);
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

void Deserializer::readBlock(char *data, int len)
{
	memcpy(data, this->data + p, len);
	p += len;
}

Deserializer &Deserializer::operator >> (uint8_t &data)
{
	data = readChar();
	return *this;
}

Deserializer &Deserializer::operator >> (uint16_t &data)
{
	readBlock((char *) &data, 2);
	return *this;
}

Deserializer &Deserializer::operator >> (uint32_t &data)
{
	readBlock((char *) &data, 4);
	return *this;
}

Deserializer &Deserializer::operator >> (uint64_t &data)
{
	readBlock((char *) &data, 8);
	return *this;
}

Deserializer &Deserializer::operator >> (int8_t &data)
{
	data = readChar();
	return *this;
}

Deserializer &Deserializer::operator >> (int16_t &data)
{
	readBlock((char *) &data, 2);
	return *this;
}

Deserializer &Deserializer::operator >> (int32_t &data)
{
	readBlock((char *) &data, 4);
	return *this;
}

Deserializer &Deserializer::operator >> (int64_t &data)
{
	readBlock((char *) &data, 8);
	return *this;
}
