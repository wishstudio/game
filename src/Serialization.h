#pragma once

/* NOTE: Use little-endian */

struct SerializerBlock;
class Serializer
{
public:
	Serializer();
	virtual ~Serializer();

	int getData(void **data);
	int getCompressedData(void **data);

	Serializer &operator << (const uint8_t data);
	Serializer &operator << (const uint16_t data);
	Serializer &operator << (const uint32_t data);
	Serializer &operator << (const uint64_t data);
	Serializer &operator << (const int8_t data);
	Serializer &operator << (const int16_t data);
	Serializer &operator << (const int32_t data);
	Serializer &operator << (const int64_t data);

private:
	void appendBlock();
	void writeChar(const char ch);
	void writeBlock(const char *data, uint32_t len);

	SerializerBlock *first, *current;
	uint32_t p, len;
};

class Deserializer
{
public:
	Deserializer(const void *data, int len, bool isCompressed = false);
	virtual ~Deserializer();

	Deserializer &operator >> (uint8_t &data);
	Deserializer &operator >> (uint16_t &data);
	Deserializer &operator >> (uint32_t &data);
	Deserializer &operator >> (uint64_t &data);
	Deserializer &operator >> (int8_t &data);
	Deserializer &operator >> (int16_t &data);
	Deserializer &operator >> (int32_t &data);
	Deserializer &operator >> (int64_t &data);

private:
	char readChar();
	void readBlock(char *data, int len);

	bool ownData;
	const char *data;
	uint32_t p, len;
};
