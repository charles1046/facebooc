#ifndef MODELS_ACCOUNT_H
#define MODELS_ACCOUNT_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

#include "basic_string.h"
#include "list.h"

typedef struct Account {
	int32_t id;
	int createdAt;

	Basic_string* name;
	Basic_string* email;
	Basic_string* username;
} Account;

typedef struct Accounts {
	Account* a;
	List list;
} Accounts;

// Account ctor
// Stores the raw string
Account* accountNew(int id, int createdAt, const char* name, const char* email,
					const char* username);

Account* accountNew_move(int id, int createdAt, Basic_string* name, Basic_string* email,
						 Basic_string* username);

// Create a new account into db
Account* accountCreate(sqlite3* db, const Basic_string* name, const Basic_string* email,
					   const Basic_string* username, const Basic_string* password);
// Get account by uid, return NULL if not found
Account* accountGetById(sqlite3* db, int uid);
// Get account by email, return NULL if not found
Account* accountGetByEmail(sqlite3* db, const Basic_string* email);
// Get account by sid, return NULL if not found
Account* accountGetBySId(sqlite3* db, const Basic_string* sid);
// Find name or email or username from db
// Return an Accounts which contains a list claiming empty if it is not found
//
// Detail: @page is the OFFSET you had viewed, because of the performance issue
// 		   we search a page (10 entries) once.
Accounts* accountSearch(sqlite3* db, const Basic_string* query, int page);
// Check if username had been taken
bool accountCheckUsername(sqlite3* db, const Basic_string* username);
// Check if email had been taken
bool accountCheckEmail(sqlite3* db, const Basic_string* email);
// Return true if authentication successful, false o.w.
bool account_auth(sqlite3* db, const Basic_string* username, const Basic_string* password);
// Account dtor, pass NULL is safe
void accountDel(Account* acc);

void accounts_delete(Accounts* as);

int accounts_is_empty(const Accounts* as);

#endif
