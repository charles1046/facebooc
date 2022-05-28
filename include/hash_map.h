#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include "pair.h"
#include <stdbool.h>
#include <stdlib.h>

// The key is immutable

// The key is const char *
// The value is void *
typedef struct Hash_map Hash_map;

Hash_map* Hash_map_new();
void Hash_map_delete(Hash_map* map);

// Return key mapped value
// Return NULL if not found
void* Hash_map_get(const Hash_map* map, const char* key);

// Return 0 if p is in the map or OOM
_Bool Hash_map_insert(Hash_map* map, const SPair* p);

// Return 0 if p is in the map or OOM
_Bool Hash_map_insert_move(Hash_map* map, SPair* p);

// It is quite expensive, return 0 if it failed
// It key is not exist in the map, return true
_Bool Hash_map_erase(Hash_map* map, const char* key);

int Hash_map_size(const Hash_map* map);

#endif
