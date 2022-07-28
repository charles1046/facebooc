#ifndef MODELS_POST_H
#define MODELS_POST_H

#include <sqlite3.h>

#include "basic_string.h"
#include "list.h"

#define MAX_BODY_LEN 65536

typedef struct Post {
	int id;
	int createdAt;
	int authorId;

	Basic_string* body;
} Post;

typedef struct Posts {
	Post* p;
	List list;
} Posts;

// Init a post struct
Post* postNew(int id, int createdAt, int authorId, const char* body);
Post* postNew_move(int id, int createdAt, int authorId, Basic_string* body);

// Insert a post to db
void postCreate_move(sqlite3* db, int uid, Basic_string* body);
Post* postGetById(sqlite3* db, int id);
Posts* postGetLatest(sqlite3* db, int authorId, int page);
Posts* postGetLatestGraph(sqlite3*, int authorId, int page);
void postDel(Post* p);

// Posts' destructor
void Posts_delete(Posts* ps);

int Posts_is_empty(const Posts* ps);

#endif
