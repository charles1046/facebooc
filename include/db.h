#ifndef __DB_H__
#define __DB_H__

typedef struct sqlite3 sqlite3;

// The Facebooc struct share the same DB as here.
// This is an encpasulated databae
void initDB(void);
sqlite3* get_db(void);
void db_close();

#endif