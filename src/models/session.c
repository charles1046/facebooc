#include "models/session.h"
#include "base64.h"
#include "basic_string.h"
#include "utility.h"

#include <string.h>
#include <time.h>

Session *sessionNew(int id, int createdAt, int accountId, const char *sessionId)
{
    Session *session = malloc(sizeof(Session));

    session->id = id;
    session->createdAt = createdAt;
    session->accountId = accountId;
    session->sessionId = strdup(sessionId);

    return session;
}

Session *sessionGetBySId(sqlite3 *DB, const char *sid)
{
    Session *session = NULL;
    sqlite3_stmt *statement = NULL;

    if (sqlite3_prepare_v2(DB,
                           "SELECT id, createdAt, account, session"
                           "  FROM sessions"
                           " WHERE session = ?",
                           -1, &statement, NULL) != SQLITE_OK) {
        return NULL;
    }

    if (sqlite3_bind_text(statement, 1, sid, -1, NULL) != SQLITE_OK)
        goto fail;

    if (sqlite3_step(statement) == SQLITE_ROW) {
        session = sessionNew(sqlite3_column_int(statement, 0),
                             sqlite3_column_int(statement, 1),
                             sqlite3_column_int(statement, 2),
                             (const char *) sqlite3_column_text(statement, 3));
    }

fail:
    sqlite3_finalize(statement);
    return session;
}

Session *sessionCreate(sqlite3 *DB, const char *username, const char *password)
{
    Session *session = NULL;
    sqlite3_stmt *statement = NULL;

    if (sqlite3_prepare_v2(DB,
                           "SELECT id"
                           "  FROM accounts"
                           " WHERE username = ?"
                           "   AND password = ?",
                           -1, &statement, NULL) != SQLITE_OK) {
        return NULL;
    }

    const Basic_string orig_pw = {.data = (char *) password,
                                  .size = strlen(password)};
    char hashed_pw[65];
    sha256_string(hashed_pw, &orig_pw);

    Basic_string *username_encoded = base64_encode(username, strlen(username));

    if (sqlite3_bind_text(statement, 1, username_encoded->data, -1, NULL) !=
        SQLITE_OK)
        goto username_encoded_dtor;
    if (sqlite3_bind_text(statement, 2, hashed_pw, -1, NULL) != SQLITE_OK)
        goto fail;
    if (sqlite3_step(statement) != SQLITE_ROW)
        goto fail;

    int aid = sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);

    if (sqlite3_prepare_v2(DB,
                           "INSERT INTO sessions(createdAt, account, session)"
                           "     VALUES         (        ?,       ?,       ?)",
                           -1, &statement, NULL) != SQLITE_OK) {
        goto fail;
    }

    if (sqlite3_bind_int(statement, 1, time(NULL)) != SQLITE_OK)
        goto fail;
    if (sqlite3_bind_int(statement, 2, aid) != SQLITE_OK)
        goto fail;

    Basic_string *sid = gen_random_dummy_string(64);
    if (sqlite3_bind_text(statement, 3, sid->data, -1, NULL) != SQLITE_OK)
        goto sid_dtor;

    if (sqlite3_step(statement) != SQLITE_DONE)
        goto sid_dtor;

    session = sessionGetBySId(DB, sid->data);

sid_dtor:
    Basic_string_delete(sid);

fail:
username_encoded_dtor:
    Basic_string_delete(username_encoded);

    sqlite3_finalize(statement);

    return session;
}

void sessionDel(Session *session)
{
    free(session->sessionId);
    free(session);
}
