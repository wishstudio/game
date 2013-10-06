#pragma once

/* NOTE: Use little-endian */

struct SerializerBlock;
class Serializer
{
public:
	Serializer();
	virtual ~Serializer();

	u32 getLength() const { return len; }
	void *getData();

	Serializer &operator << (const u8 data);
	Serializer &operator << (const u16 data);
	Serializer &operator << (const u32 data);
	Serializer &operator << (const u64 data);
	Serializer &operator << (const s8 data);
	Serializer &operator << (const s16 data);
	Serializer &operator << (const s32 data);
	Serializer &operator << (const s64 data);

private:
	void appendBlock();
	void writeChar(const char ch);
	void writeBlock(const char *data, u32 len);

	SerializerBlock *first, *current;
	u32 p, len;
};

class Deserializer
{
public:
	Deserializer(const void *data, u32 len);
	virtual ~Deserializer();

	Deserializer &operator >> (u8 &data);
	Deserializer &operator >> (u16 &data);
	Deserializer &operator >> (u32 &data);
	Deserializer &operator >> (u64 &data);
	Deserializer &operator >> (s8 &data);
	Deserializer &operator >> (s16 &data);
	Deserializer &operator >> (s32 &data);
	Deserializer &operator >> (s64 &data);

private:
	char readChar();
	void readBlock(char *data, u32 len);

	const char *data;
	u32 p, len;
};
