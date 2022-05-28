#ifndef __MAP__H__
#define __MAP__H__
#include "pair.h"
#include "rbtree.h"
#include <stdbool.h>
#include <stdint.h>

// This is general map using rbtree
typedef struct Map Map;

Map* Map_new();
void Map_delete(Map* m);
void* Map_get(Map* m, uint64_t key);
_Bool Map_insert(Map* m, const Pair* p);
_Bool Map_insert_move(Map* m, Pair* p);
_Bool Map_erase(Map* m, uint64_t key);
size_t Map_size(const Map* m);

// Your key is pointer type
Map* Map_P_new();
void Map_P_delete(Map* m);
void* Map_P_get(Map* m, const void* key);
_Bool Map_P_insert(Map* m, const PPair* p);
_Bool Map_P_insert_move(Map* m, PPair* p);
_Bool Map_P_erase(Map* m, void* key);
size_t Map_P_size(const Map* m);

#endif
