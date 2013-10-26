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
}

Database::~Database()
{
	sqlite3_close(db);
}

bool Database::loadChunk(Chunk *chunk)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT data FROM chunks WHERE x = ? AND y = ? AND z = ?", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, chunk->x());
	sqlite3_bind_int(stmt, 2, chunk->y());
	sqlite3_bind_int(stmt, 3, chunk->z());
	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		/* Not found */
		sqlite3_finalize(stmt);
		return false;
	}
	else
	{
		/* Found, read blob data */
		const void *data = sqlite3_column_blob(stmt, 0); /* sqlite will free this automatically */
		u32 len = sqlite3_column_bytes(stmt, 0);
		Deserializer deserializer(data, len, true);
		deserializer >> *chunk;
		sqlite3_finalize(stmt);
		return true;
	}
}

void Database::saveChunk(const Chunk *chunk)
{
	Serializer serializer;
	serializer << *chunk;

	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO chunks (x, y, z, data) VALUES (?, ?, ?, ?)", -1, &stmt, nullptr);
	sqlite3_bind_int(stmt, 1, chunk->x());
	sqlite3_bind_int(stmt, 2, chunk->y());
	sqlite3_bind_int(stmt, 3, chunk->z());
	/* Sqlite will call free() when done */
	void *data;
	u32 len = serializer.getCompressedData(&data);
	sqlite3_bind_blob(stmt, 4, data, len, free);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

void Database::beginTransaction()
{
	sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
}

void Database::commitTransaction()
{
	sqlite3_exec(db, "COMMIT TRANSACTION", nullptr, nullptr, nullptr);
}
