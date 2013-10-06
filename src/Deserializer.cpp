#include "stdafx.h"

#include "Serialization.h"

Deserializer::Deserializer(const void *data, u32 len)
{
	this->data = (const char *) data;
	this->len = len;
	this->p = 0;
}

Deserializer::~Deserializer()
{
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
