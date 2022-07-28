#ifndef MODELS_SESSION_H
#define MODELS_SESSION_H

#include <sqlite3.h>

typedef struct Session {
	int id;
	int createdAt;
	int accountId;

	char* sessionId;
} Session;

Session* sessionNew(int id, int createdAt, int accountId, const char* sessionId);
Session* sessionGetBySId(sqlite3* DB, const char* sid);

// Check if username and password is matched, return NULL if it failed
Session* sessionCreate(sqlite3* DB, const char* username, const char* password);
void sessionDel(Session* s);

#endif
