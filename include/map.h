#ifndef __MAP__H__
#define __MAP__H__

#include "pair.h"
#include "vTable.h"

struct Rbtree;

// Key is immutable
#define decl_map(T1, T2)                                                                       \
	decl_rbtree(T1, T2);                                                                       \
	typedef struct Map_##T1_##T2 Map_##T1_##T2;                                                \
	struct Map_##T1_##T2 {                                                                     \
		Rbtree##T1_##T2* this;                                                                 \
	};                                                                                         \
	inline Map_##T1_##T2* new_map_##T1_##T2(void) {                                            \
		Map_##T1_##T2* m = malloc(sizeof(Map_##T1_##T2));                                      \
		m->this = new_Rbtree_##T1_##T2(default_cmp_##T1_##T2);                                 \
		return m;                                                                              \
	}                                                                                          \
	inline Map_##T1_##T2* insert_map_##T1_##T2(Map_##T1_##T2* node, const Pair_##T1_##T2* p) { \
		return insert_Rbtree_##T1_##T2(node->this, p);                                         \
	}                                                                                          \
	inline Map_##T1_##T2* insert_map_move_##T1_##T2(Map_##T1_##T2* node, Pair_##T1_##T2* p) {  \
		return insert_Rbtree_move_##T1_##T2(node->this, p);                                    \
	}                                                                                          \
	/*Return NULL if not found*/                                                               \
	inline Pair_##T1_##T2* find_map_##T1_##T2(const Map_##T1_##T2* node, const T1* key) {      \
		return find_Rbtree_##T1_##T2(node->this, key);                                         \
	}                                                                                          \
	/*Return new root*/                                                                        \
	inline Map_##T1_##T2* erase_map_##T1_##T2(Map_##T1_##T2* node, const T1* key) {            \
		erase_Rbtree_##T1_##T2(node->entry, key);                                              \
		return node;                                                                           \
	}                                                                                          \
	/*Reserve the rbroot space of Map, you can also insert some pairs in*/                     \
	inline Map_##T1_##T2* clean_map_##T1_##T2(Map_##T1_##T2* node) {                           \
		clean_Rbtree_##T1_##T2(node->entry);                                                   \
		return node;                                                                           \
	}                                                                                          \
	/*You should not use this Map any more*/                                                   \
	inline void free_map_##T1_##T2(Map_##T1_##T2* node) {                                      \
		free_Rbtree_##T1_##T2(node->entry);                                                    \
		free(node);                                                                            \
		node = NULL;                                                                           \
	}

#endif

decl_map(int, char);
