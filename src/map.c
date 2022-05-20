#include "map.h"

#include "splay.h"
#include <cstdlib>
#include <iostream>

/* RR(Y rotates to the right):

		k2                   k1
	   /  \                 /  \
	  k1   Z     ==>       X   k2
	 / \                      /  \
	X   Y                    Y    Z
*/
inline splay* RR_Rotate(splay* k2) {
	splay* k1 = k2->left;
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
inline splay* LL_Rotate(splay* k2) {
	splay* k1 = k2->right;
	k2->right = k1->left;
	k1->left = k2;
	return k1;
}

/* An implementation of top-down splay tree
 If key is in the tree, then the node containing the key will be rotated to root,
 else the last non-NULL node (on the search path) will be rotated to root.
 */
splay* Splay(int key, splay* root) {
	if(!root)
		return NULL;
	splay header;
	/* header.right points to L tree; header.left points to R Tree */
	header.left = header.right = NULL;
	splay* left_tree_max = &header;
	splay* right_tree_min = &header;

	/* loop until root->left == NULL || root->right == NULL; then break!
	   (or when find the key, break too.)
	 The zig/zag mode would only happen when cannot find key and will reach
	 null on one side after RR or LL Rotation.
	 */
	while(1) {
		if(key < root->key) {
			if(!root->left)
				break;
			if(key < root->left->key) {
				root = RR_Rotate(root); /* only zig-zig mode need to rotate once,
										   because zig-zag mode is handled as zig
										   mode, which doesn't require rotate,
										   just linking it to R Tree */
				if(!root->left)
					break;
			}
			/* Link to R Tree */
			right_tree_min->left = root;
			right_tree_min = right_tree_min->left;
			root = root->left;
			right_tree_min->left = NULL;
		}
		else if(key > root->key) {
			if(!root->right)
				break;
			if(key > root->right->key) {
				root = LL_Rotate(root); /* only zag-zag mode need to rotate once,
										   because zag-zig mode is handled as zag
										   mode, which doesn't require rotate,
										   just linking it to L Tree */
				if(!root->right)
					break;
			}
			/* Link to L Tree */
			left_tree_max->right = root;
			left_tree_max = left_tree_max->right;
			root = root->right;
			left_tree_max->right = NULL;
		}
		else
			break;
	}
	/* assemble L Tree, Middle Tree and R tree together */
	left_tree_max->right = root->left;
	right_tree_min->left = root->right;
	root->left = header.right;
	root->right = header.left;

	return root;
}

splay* New_Node(KEY_TYPE key) {
	splay* p_node = new splay;
	if(!p_node) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	p_node->key = key;
	p_node->left = p_node->right = NULL;
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

splay* Insert(KEY_TYPE key, splay* root) {
	splay* p_node = New_Node(key);
	if(!root)
		return p_node;
	splay* parent_temp;
	splay* temp = root;
	while(temp) {
		parent_temp = temp;
		if(key <= temp->key)
			temp = temp->left;
		else
			temp = temp->right;
	}
	if(key <= parent_temp->key)
		parent_temp->left = p_node;
	else
		parent_temp->right = p_node;

	return (root = Splay(key, root));
}

/*
Implementation: just Splay(key, root), if key doesn't exist in the tree(key !=
root->key), return root directly; else join the root->left and root->right
and then free(old_root).
To join T1 and T2 (where all elements in T1 < any element in T2) is easy if we
have the largest element in T1 as T1's root, and here's a trick to simplify code,
see "Note" below.
 */
splay* Delete(KEY_TYPE key, splay* root) {
	splay* temp;
	if(!root)
		return NULL;
	root = Splay(key, root);
	if(key != root->key)  // No such node in splay tree
		return root;
	else {
		if(!root->left) {
			temp = root;
			root = root->right;
		}
		else {
			temp = root;
			/*Note: Since key == root->key, so after Splay(key, root->left),
			  the tree we get will have no right child tree. (key > any key in
			  root->left)*/
			root = Splay(key, root->left);
			root->right = temp->right;
		}
		free(temp);
		return root;
	}
}

splay* Search(KEY_TYPE key, splay* root) {
	return Splay(key, root);
}

void InOrder(splay* root) {
	if(root) {
		InOrder(root->left);
		std::cout << "key: " << root->key;
		if(root->left)
			std::cout << " | left child: " << root->left->key;
		if(root->right)
			std::cout << " | right child: " << root->right->key;
		std::cout << "\n";
		InOrder(root->right);
	}
}