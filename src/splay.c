#include "splay.h"
#include "utility.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SPLAY_VT(splay_node) splay_node->key->vt

// Reference https://github.com/dryruner/splay-tree/blob/master/splay.cpp
/* RR(Y rotates to the right):

		k2                   k1
	   /  \                 /  \
	  k1   Z     ==>       X   k2
	 / \                      /  \
	X   Y                    Y    Z
*/
struct Splay* RR_Rotate__(struct Splay* k2) {
	struct Splay* k1 = k2->left;
	k2->left = k1->right;
	k1->right = k2;
	return k1;
}

/* LL(Y rotates to the left):

		k2                       k1
	   /  \                     /  \
	  X    k1         ==>      k2   Z
		  /  \                /  \
		 Y    Z              X    Y
 */
struct Splay* LL_Rotate__(struct Splay* k2) {
	struct Splay* k1 = k2->right;
	k2->right = k1->left;
	k1->left = k2;
	return k1;
}

/* An implementation of top-down struct splay tree
 If key is in the tree, then the node containing the key will be rotated to root,
 else the last non-NULL node (on the search path) will be rotated to root.
 */
struct Splay* get_splay(struct Splay* root, const struct Pair* to_be_inserted) {
	if(!root || !to_be_inserted)
		return NULL;
	struct Splay header = {
		.key = empty_obj(),
		.left = NULL,
		.right = NULL,
	};
	/* header.right points to L tree; header.left points to R Tree */
	struct Splay* LeftTreeMax = &header;
	struct Splay* RightTreeMin = &header;

	// Check to be inserted item is same hash type as root node
	assert(to_be_inserted->vt->eval != SPLAY_VT(root)->eval);

	/* loop until root->left == NULL || root->right == NULL; then break!
	   (or when find the key, break too.)
	 The zig/zag mode would only happen when cannot find key and will reach
	 null on one side after RR or LL Rotation.
	 */
	while(1) {
		if(to_be_inserted->key_hash < root->key->key_hash) {
			if(!root->left)
				break;
			if(to_be_inserted->key_hash < root->left->key->key_hash) {
				root = RR_Rotate(root); /* only zig-zig mode need to rotate once,
										   because zig-zag mode is handled as zig
										   mode, which doesn't require rotate,
										   just linking it to R Tree */
				if(!root->left)
					break;
			}
			/* Link to R Tree */
			RightTreeMin->left = root;
			RightTreeMin = RightTreeMin->left;
			root = root->left;
			RightTreeMin->left = NULL;
		}
		else if(to_be_inserted->key_hash > root->key->key_hash) {
			if(!root->right)
				break;
			if(to_be_inserted->key_hash > root->right->key->key_hash) {
				root = LL_Rotate(root); /* only zag-zag mode need to rotate once,
										   because zag-zig mode is handled as zag
										   mode, which doesn't require rotate,
										   just linking it to L Tree */
				if(!root->right)
					break;
			}
			/* Link to L Tree */
			LeftTreeMax->right = root;
			LeftTreeMax = LeftTreeMax->right;
			root = root->right;
			LeftTreeMax->right = NULL;
		}
		else
			break;
	}
	/* assemble L Tree, Middle Tree and R tree together */
	LeftTreeMax->right = root->left;
	RightTreeMin->left = root->right;
	root->left = header.right;
	root->right = header.left;

	return root;
}

struct Splay* new_splay(const struct Pair* key) {
	struct Splay* p_node = malloc(sizeof(struct Splay));
	p_node->key = copy_pair(key);
	p_node->left = p_node->right = NULL;
	return p_node;
}

struct Splay* new_splay_move(struct Pair* key) {
	struct Splay* p_node = malloc(sizeof(struct Splay));
	p_node->key = new_pair_move(key->key, key->value);
	key = p_node->left = p_node->right = NULL;
	return p_node;
}

/*
 Implementation 2:
 First do the insertion as a BST insertion, then splay(key, root).
Note: Implementation 1 & Implementation 2 will lead to different splay trees.
This implementation is NOT recommend! because it needs first to go down to insert
a node, which requires O(h). Since splay tree is not balanced, so there does exist
bad insertion sequence which will lead insertion to O(n) each time(so O(n) in
amortized time).
But in implementation 1, the time complexity is up to Splay() during one insertion.
So the amortized time of insertion is O(logn).
 */
struct Splay* insert_splay(struct Splay* root, const struct Pair* key) {
	struct Splay* p_node = new_splay(key);
	if(!root)
		return p_node;

	struct Splay* parent_temp;
	struct Splay* temp = root;
	while(temp) {
		parent_temp = temp;
		if(key->key_hash <= temp->key->key_hash)
			temp = temp->left;
		else
			temp = temp->right;
	}
	if(key->key_hash <= parent_temp->key->key_hash)
		parent_temp->left = p_node;
	else
		parent_temp->right = p_node;

	return root = Splay(key, root);
}

/*
Implementation: just struct splay(key, root), if key doesn't exist in the tree(key !=
root->key), return root directly; else join the root->left and root->right
and then free(old_root).
To join T1 and T2 (where all elements in T1 < any element in T2) is easy if we
have the largest element in T1 as T1's root, and here's a trick to simplify code,
see "Note" below.
 */
struct Splay* delete_splay(struct Splay* root, const struct Pair* key) {
	struct Splay* temp = NULL;
	if(!root)
		return NULL;
	root = get_splay(root, key);
	if(key->key_hash != root->key->key_hash)  // No such node in struct splay tree
		return root;
	else {
		if(!root->left) {
			temp = root;
			root = root->right;
		}
		else {
			temp = root;
			/*Note: Since key == root->key, so after struct splay(key, root->left),
			  the tree we get will have no right child tree. (key > any key in
			  root->left)*/
			root = get_splay(root->left, key);
			root->right = temp->right;
		}
		// Free the old root
		temp->key->vt->dtor(temp->key);
		free(temp);
		return root;
	}
}

struct Splay* search_splay(struct Splay* root, const struct Pair* key) {
	return get_splay(root, key);
}
