#include <time.h>

#include "models/post.h"

static inline Posts* Posts_new(void);

Post* postNew(int id, int createdAt, int authorId, const char* body) {
	Post* post = malloc(sizeof(Post));

	post->id = id;
	post->createdAt = createdAt;
	post->authorId = authorId;
	post->body = Basic_string_init(body);

	return post;
}

Post* postNew_move(int id, int createdAt, int authorId, Basic_string* body) {
	Post* post = malloc(sizeof(Post));

	post->id = id;
	post->createdAt = createdAt;
	post->authorId = authorId;
	post->body = body;

	body = NULL;

	return post;
}

void postCreate_move(sqlite3* DB, int authorId, Basic_string* body) {
	sqlite3_stmt* statement = NULL;

	int rc = sqlite3_prepare_v2(DB,
								"INSERT INTO posts(createdAt, author, body)"
								"     VALUES      (        ?,      ?,    ?)",
								-1, &statement, NULL);

	if(rc != SQLITE_OK)
		return;

	html_escape_trans(body);

	if(sqlite3_bind_int(statement, 1, time(NULL)) != SQLITE_OK)
		goto fail;
	if(sqlite3_bind_int(statement, 2, authorId) != SQLITE_OK)
		goto fail;
	if(sqlite3_bind_text(statement, 3, body->data, -1, NULL) != SQLITE_OK)
		goto fail;

	if(sqlite3_step(statement) == SQLITE_DONE)
		sqlite3_last_insert_rowid(DB);

fail:
	Basic_string_delete(body);
	sqlite3_finalize(statement);
}

Post* postGetById(sqlite3* DB, int id) {
	if(id == -1)
		return NULL;

	Post* post = NULL;
	sqlite3_stmt* statement;

	if(sqlite3_prepare_v2(DB,
						  "SELECT id, createdAt, author, body"
						  "  FROM posts"
						  " WHERE id = ?",
						  -1, &statement, NULL)
	   != SQLITE_OK) {
		return NULL;
	}

	if(sqlite3_bind_int(statement, 1, id) != SQLITE_OK)
		goto fail;
	if(sqlite3_step(statement) != SQLITE_ROW)
		goto fail;

	Basic_string* body = Basic_string_init((const char*)sqlite3_column_text(statement, 3));
	newline_to_br(body);

	post = postNew_move(sqlite3_column_int(statement, 0), sqlite3_column_int(statement, 1),
						sqlite3_column_int(statement, 2), body);

fail:
	sqlite3_finalize(statement);
	return post;
}

Posts* postGetLatest(sqlite3* DB, int accountId, int page) {
	if(accountId == -1)
		return NULL;

	Posts* ps = Posts_new();
	sqlite3_stmt* statement = NULL;

	int rc = sqlite3_prepare_v2(DB,
								"SELECT id, createdAt, author, body"
								"  FROM posts"
								" WHERE author = ?"
								" ORDER BY createdAt DESC"
								" LIMIT 10 "
								"OFFSET ?",
								-1, &statement, NULL);

	if(rc != SQLITE_OK)
		return NULL;
	if(sqlite3_bind_int(statement, 1, accountId) != SQLITE_OK)
		goto fail;
	if(sqlite3_bind_int(statement, 2, page * 10) != SQLITE_OK)
		goto fail;

	while(sqlite3_step(statement) == SQLITE_ROW) {
		Basic_string* tmp_body = Basic_string_init((const char*)sqlite3_column_text(statement, 3));
		newline_to_br(tmp_body);

		Posts* new_ps = Posts_new();
		new_ps->p = postNew_move(sqlite3_column_int(statement, 0), sqlite3_column_int(statement, 1),
								 sqlite3_column_int(statement, 2), tmp_body);

		List_insert_tail(&ps->list, &new_ps->list);
	}

fail:
	sqlite3_finalize(statement);
	return ps;
}

Posts* postGetLatestGraph(sqlite3* DB, int accountId, int page) {
	if(accountId == -1)
		return NULL;

	Posts* ps = Posts_new();
	sqlite3_stmt* statement = NULL;

	int rc = sqlite3_prepare_v2(DB,
								"SELECT posts.id, posts.createdAt, posts.author, posts.body"
								"  FROM posts"
								"  LEFT OUTER JOIN connections"
								"    ON posts.author = connections.account2"
								" WHERE connections.account1 = ?"
								"    OR posts.author = ?"
								" ORDER BY posts.createdAt DESC"
								" LIMIT 10 "
								"OFFSET ?",
								-1, &statement, NULL);

	if(rc != SQLITE_OK)
		return NULL;
	if(sqlite3_bind_int(statement, 1, accountId) != SQLITE_OK)
		goto fail;
	if(sqlite3_bind_int(statement, 2, accountId) != SQLITE_OK)
		goto fail;
	if(sqlite3_bind_int(statement, 3, page * 10) != SQLITE_OK)
		goto fail;

	while(sqlite3_step(statement) == SQLITE_ROW) {
		Basic_string* tmp_body = Basic_string_init((const char*)sqlite3_column_text(statement, 3));
		newline_to_br(tmp_body);

		Posts* new_ps = Posts_new();
		new_ps->p = postNew_move(sqlite3_column_int(statement, 0), sqlite3_column_int(statement, 1),
								 sqlite3_column_int(statement, 2), tmp_body);

		List_insert_tail(&ps->list, &new_ps->list);
	}

fail:
	sqlite3_finalize(statement);
	return ps;
}

void postDel(Post* post) {
	if(!post)
		return;

	Basic_string_delete(post->body);
	free(post);
	post = NULL;
}

static inline Posts* Posts_new(void) {
	Posts* ps = malloc(sizeof(Posts));
	ps->p = NULL;

	List_ctor(&ps->list);

	return ps;
}

void Posts_delete(Posts* ps) {
	if(unlikely(!ps))
		return;

	List *node = NULL, *safe = NULL;
	list_for_each_safe(node, safe, &ps->list) {
		Posts* ps_node = container_of(node, Posts, list);
		postDel(ps_node->p);
		free(ps_node);
	}

	// Delete head, which is an empty node
	postDel(ps->p);
	free(ps);
	ps = NULL;
}

int Posts_is_empty(const Posts* ps) {
	return List_is_empty(&ps->list);
}