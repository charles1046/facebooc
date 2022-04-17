#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

Node* insert(const void* value, size_t size, const Node* next) {
	Node* new_node = malloc(sizeof(Node));
	assert(new_node);

	*(void**)(&new_node->value) = malloc(size);	 // Bypass const
	assert(new_node->value);

	new_node->next = (Node*)next;
	*(size_t*)(&new_node->size) = size;	 // Bypass const

	memcpy((void*)new_node->value, value, size);

	return new_node;
}

void clear(Node* node) {
	while(node != NULL) {
		const Node* tmp = node->next;
		free((void*)node->value);
		free(node);
		node = (Node*)tmp;
	}
}

_Bool listForEach(Node* node, List_op func) {
	_Bool result = true;
	Node** indrect = &node;

	while(*indrect != NULL && result) {
		result = func((void*)(*indrect)->value);  // Do it
		indrect = (Node**)&((*indrect)->next);	  // To next
	}

	return result;
}

Node* reverse(Node* head) {
	Node* cur = head;
	Node* prev = NULL;

	while(cur) {
		Node* tmp = cur->next;
		cur->next = prev;
		prev = cur;
		cur = tmp;
	}

	return prev;
}