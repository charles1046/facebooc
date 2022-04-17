#ifndef KV_H
#define KV_H

#include <stdbool.h>

#include "list.h"

// TODO: Use container of macro to rewrite this
// TODO: Use `map' to replace list dependency
typedef struct KV {
	char* key;
	char* value;
} KV;

KV* kvNew(const char* key, const char* value);
void kvDel(KV*);
// Delete a list which is made by kv
void kvDelList(Node* head);
void kvPrintList(Node* head);
char* kvFindList(const Node* head, const char* key);

#endif
