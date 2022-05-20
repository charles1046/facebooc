#ifndef __SPLAY__H__
#define __SPLAY__H__

#include "pair.h"

// An implementation of a splay tree
// Those detail please refer to https://github.com/dryruner/splay-tree
#define decl_splay(T1, T2)                                                                    \
	decl_pair(T1, T2);                                                                        \
	typedef strcut Splay_##T1_##T2 Splay_##T1_##T2;                                           \
	struct Splay_##T1_##T2 {                                                                  \
		Pair_##T1_##T2* key;                                                                  \
		Splay_##T1_##T2* left;                                                                \
		Splay_##T1_##T2* right;                                                               \
	};                                                                                        \
	/*! Don't use, this is a internal function */                                             \
	/*! RR(Y rotates to the right):                                                           \
		k2                   k1                                                               \
	   /  \                 /  \                                                              \
	  k1   Z     ==>       X   k2                                                             \
	 / \                      /  \                                                            \
	X   Y                    Y    Z */                                                        \
	Splay_##T1_##T2* RR_Rotate__(Splay_##T1_##T2* k2) {                                       \
		Splay_##T1_##T2* k1 = k2->left;                                                       \
		k2->left = k1->right;                                                                 \
		k1->right = k2;                                                                       \
		return k1;                                                                            \
	}                                                                                         \
	/*! Don't use, this is a internal function */                                             \
	/* LL(Y rotates to the left):                                                             \
	  k2                       k1                                                             \
	 /  \                     /  \                                                            \
	X    k1         ==>      k2   Z                                                           \
		/  \                /  \                                                              \
	   Y    Z              X    Y */                                                          \
	Splay_##T1_##T2* LL_Rotate__(Splay_##T1_##T2* k2) {                                       \
		Splay_##T1_##T2* k1 = k2->right;                                                      \
		k2->right = k1->left;                                                                 \
		k1->left = k2;                                                                        \
		return k1;                                                                            \
	}                                                                                         \
	Splay_##T1_##T2* new_splay_##T1_##T2(const Pair_##T1_##T2* p) {                           \
		Splay_##T1_##T2* p_node = malloc(sizeof(Splay_##T1_##T2));                            \
		p_node->key = copy_ctor_##T1_##T2(p->key, p->value);                                  \
		p_node->left = p_node->right = NULL;                                                  \
		return p_node;                                                                        \
	}                                                                                         \
	Splay_##T1_##T2* new_splay_move_##T1_##T2(Pair_##T1_##T2* p) {                            \
		Splay_##T1_##T2* p_node = malloc(sizeof(Splay_##T1_##T2));                            \
		p_node->key = move_ctor_##T1_##T2(p->key, p->value);                                  \
		p = p_node->left = p_node->right = NULL;                                              \
		return p_node;                                                                        \
	}                                                                                         \
	Splay_##T1_##T2* get_splay_##T1_##T2(Splay_##T1_##T2* root, const Pair_##T1_##T2* p) {    \
		if(!root || !p)                                                                       \
			return NULL;                                                                      \
		Splay_##T1_##T2 header = { .key = NULL, .left = NULL, .right = NULL };                \
		Splay_##T1_##T2* left_tree_max = &header;                                             \
		Splay_##T1_##T2* right_tree_min = &header;                                            \
		;                                                                                     \
		while(1) {                                                                            \
			if(p->key < root->key->key) {                                                     \
				if(!root->left)                                                               \
					break;                                                                    \
				if(p->key < root->left->key->key) {                                           \
					root = RR_Rotate__(root);                                                 \
					if(!root->left)                                                           \
						break;                                                                \
				};                                                                            \
				/* Link to R Tree */;                                                         \
				right_tree_min->left = root;                                                  \
				right_tree_min = right_tree_min->left;                                        \
				root = root->left;                                                            \
				right_tree_min->left = NULL;                                                  \
			}                                                                                 \
			else if(p->key > root->key->key) {                                                \
				if(!root->right)                                                              \
					break;                                                                    \
				if(p->key > root->right->key->key) {                                          \
					root = LL_Rotate__(root);                                                 \
					if(!root->right)                                                          \
						break;                                                                \
				};                                                                            \
				/* Link to L Tree */                                                          \
				left_tree_max->right = root;                                                  \
				left_tree_max = left_tree_max->right;                                         \
				root = root->right;                                                           \
				left_tree_max->right = NULL;                                                  \
			}                                                                                 \
			else                                                                              \
				break;                                                                        \
		}                                                                                     \
		/* assemble L Tree, Middle Tree and R tree together */                                \
		left_tree_max->right = root->left;                                                    \
		right_tree_min->left = root->right;                                                   \
		root->left = header.right;                                                            \
		root->right = header.left;                                                            \
		return root;                                                                          \
	}                                                                                         \
	Splay_##T1_##T2* insert_splay_##T1_##T2(Splay_##T1_##T2* root, const Pair_##T1_##T2* p) { \
		Splay_##T1_##T2* p_node = new_splay_##T1_##T2(p);                                     \
		if(!root)                                                                             \
			return p_node;                                                                    \
		Splay_##T1_##T2* parent_temp = NULL;                                                  \
		Splay_##T1_##T2* temp = root;                                                         \
		while(temp) {                                                                         \
			parent_temp = temp;                                                               \
			if(p->key <= temp->key->key)                                                      \
				temp = temp->left;                                                            \
			else                                                                              \
				temp = temp->right;                                                           \
		}                                                                                     \
		if(p->key <= parent_temp->key->key)                                                   \
			parent_temp->left = p_node;                                                       \
		else                                                                                  \
			parent_temp->right = p_node;                                                      \
		root = get_splay_##T1_##T2(root, p);                                                  \
		return root;                                                                          \
	}                                                                                         \
	Splay_##T1_##T2* insert_splay__move_##T1_##T2(Splay_##T1_##T2* root, Pair_##T1_##T2* p) { \
		Splay_##T1_##T2* p_node = new_splay_move_##T1_##T2(p);                                \
		if(!root)                                                                             \
			return p_node;                                                                    \
		Splay_##T1_##T2* parent_temp = NULL;                                                  \
		Splay_##T1_##T2* temp = root;                                                         \
		while(temp) {                                                                         \
			parent_temp = temp;                                                               \
			if(p->key <= temp->key->key)                                                      \
				temp = temp->left;                                                            \
			else                                                                              \
				temp = temp->right;                                                           \
		}                                                                                     \
		if(p->key <= parent_temp->key->key)                                                   \
			parent_temp->left = p_node;                                                       \
		else                                                                                  \
			parent_temp->right = p_node;                                                      \
		root = get_splay_##T1_##T2(root, p);                                                  \
		return root;                                                                          \
	}                                                                                         \
	Splay_##T1_##T2* delete_splay_##T1_##T2(Splay_##T1_##T2* root, const Pair_##T1_##T2* p) { \
		if(!root)                                                                             \
			return NULL;                                                                      \
		root = get_splay_##T1_##T2(root, p);                                                  \
		;                                                                                     \
	}                                                                                         \
	Splay_##T1_##T2* find_splay_##T1_##T2(Splay_##T1_##T2* root, const Pair_##T1_##T2* p);

struct Splay {
	const struct Pair* key;
	struct Splay* left;
	struct Splay* right;
};

struct Splay* new_splay(const struct Pair* key);
struct Splay* new_splay_move(struct Pair* key);
struct Splay* get_splay(struct Splay* root, const struct Pair* key);
struct Splay* insert_splay(struct Splay* root, const struct Pair* key);
struct Splay* delete_splay(struct Splay* root, const struct Pair* key);
struct Splay* search_splay(struct Splay* root, const struct Pair* key);
#endif