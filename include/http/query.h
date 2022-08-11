#ifndef __QUERY_H__
#define __QUERY_H__

typedef struct Query Query;

// Suppose it is end by '\0'
Query *query_parser(const char *path);
const char *query_get(const Query *restrict q, const char *restrict key);
void query_delete(Query *q);
void query_delete_const(const Query *q);

#define query_delete(q) \
    _Generic((q), const Query * : query_delete_const, Query * : query_delete)(q)

#endif
