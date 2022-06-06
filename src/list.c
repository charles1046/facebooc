#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

Node* insert(const void* restrict value, size_t size, const Node* restrict next) {
	Node* new_node = malloc(sizeof(Node));
	*(void**)(&new_node->value) = malloc(size);	 // Bypass const

	new_node->next = (Node*)next;
	*(size_t*)(&new_node->size) = size;	 // Bypass const

	memcpy((void*)new_node->value, value, size);

	return new_node;
}

Node* insert_move(void* restrict value, const Node* restrict next) {
	Node* new_node = malloc(sizeof(Node));
	*(void**)(&new_node->value) = value;
	new_node->next = (Node*)next;
	value = NULL;

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
	Node** indirect = &node;
	while((*indirect) != NULL && result) {
		result = func((void*)(*indirect)->value);  // Do it
		indirect = (Node**)&((*indirect)->next);   // To next
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