#pragma once

struct sqlite3;
struct sqlite3_stmt;
class Chunk;
class Database
{
public:
	Database();
	virtual ~Database();
	
	bool loadChunk(Chunk *chunk);
	void saveChunk(const Chunk *chunk);
	
	void beginTransaction();
	void commitTransaction();
	
private:
	sqlite3 *db;
	sqlite3_stmt *chunkLoadStatement, *chunkSaveStatement;
};
