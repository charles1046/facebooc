#ifndef MODELS_ACCOUNT_H
#define MODELS_ACCOUNT_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

#include "list.h"

typedef struct Account {
	int32_t id;
	int createdAt;

	char* name;
	char* email;
	char* username;
} Account;

// !FIXME: Write comments, what do they do?
// !Please notice the uid is int32_t
Account* accountNew(int, int, const char*, const char*, const char*);
Account* accountCreate(sqlite3*, const char* name, const char* email, const char* username,
					   const char* password);
Account* accountGetById(sqlite3*, int);
Account* accountGetByEmail(sqlite3*, const char*);
Account* accountGetBySId(sqlite3*, const char*);
Node* accountSearch(sqlite3*, const char*, int);
bool accountCheckUsername(sqlite3*, const char* username);
bool accountCheckEmail(sqlite3*, const char* email);
void accountDel(Account*);

#endif
