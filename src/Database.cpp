#include "stdafx.h"

#include <sqlite3.h>
#include "Chunk.h"
#include "Database.h"
#include "Serialization.h"

Database::Database()
{
	sqlite3_open("data", &db);

	/* Create database structure if not exists */
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS chunks (x INTEGER NOT NULL, y INTEGER NOT NULL, z INTEGER NOT NULL, data BLOB NOT NULL, PRIMARY KEY (x, y, z))", 0, 0, 0);

	/* Prepare statements */
	sqlite3_prepare_v2(db, "SELECT data FROM chunks WHERE x = ? AND y = ? AND z = ?", -1, &chunkLoadStatement, nullptr);
	sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?)", -1, &chunkSaveStatement, nullptr);
}

Database::~Database()
{
	sqlite3_finalize(chunkLoadStatement);
	sqlite3_finalize(chunkSaveStatement);
	sqlite3_close(db);
}

bool Database::loadChunk(Chunk *chunk)
{
	sqlite3_reset(chunkLoadStatement);
	sqlite3_bind_int(chunkLoadStatement, 1, chunk->x());
	sqlite3_bind_int(chunkLoadStatement, 2, chunk->y());
	sqlite3_bind_int(chunkLoadStatement, 3, chunk->z());
	if (sqlite3_step(chunkLoadStatement) == SQLITE_DONE)
	{
		/* Not found */
		return false;
	}
	else
	{
		/* Found, read blob data */
		const void *data = sqlite3_column_blob(chunkLoadStatement, 0); /* sqlite will free this automatically */
		u32 len = sqlite3_column_bytes(chunkLoadStatement, 0);
		Deserializer deserializer(data, len, true);
		deserializer >> *chunk;
		return true;
	}
}

void Database::saveChunk(const Chunk *chunk)
{
	Serializer serializer;
	serializer << *chunk;

	sqlite3_reset(chunkSaveStatement);
	sqlite3_bind_int(chunkSaveStatement, 1, chunk->x());
	sqlite3_bind_int(chunkSaveStatement, 2, chunk->y());
	sqlite3_bind_int(chunkSaveStatement, 3, chunk->z());
	/* Sqlite will call free() when done */
	void *data;
	u32 len = serializer.getCompressedData(&data);
	sqlite3_bind_blob(chunkSaveStatement, 4, data, len, free);
	sqlite3_step(chunkSaveStatement);
}

void Database::beginTransaction()
{
	sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
}

void Database::commitTransaction()
{
	sqlite3_exec(db, "COMMIT TRANSACTION", nullptr, nullptr, nullptr);
}
