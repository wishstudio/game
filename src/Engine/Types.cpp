#include <stdafx.h>

static const u32 ELEMENT_TYPE_SIZE[TYPE_COUNT] = {
	1, 2, 3, 4, /* BYTE */
	1, 2, 3, 4, /* BYTE_NORM */
	1, 2, 3, 4, /* UBYTE */
	1, 2, 3, 4, /* UBYTE_NORM */
	2, 4, 6, 8, /* SHORT */
	2, 4, 6, 8, /* SHORT_NORM */
	2, 4, 6, 8, /* USHORT */
	2, 4, 6, 8, /* USHORT_NORM */
	4, 8, 12, 16, /* INT */
	4, 8, 12, 16, /* UINT */
	4, 8, 12, 16, /* FLOAT */
	8, 16, 24, 32, /* DOUBLE */
};

u32 getTypeSize(VertexElementType type)
{
	return ELEMENT_TYPE_SIZE[type];
}
