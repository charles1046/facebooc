#include <stdio.h>
#include <string.h>

#include "bs.h"
#include "kv.h"

// Self implentation
static inline char* strndup(const char* str, size_t len) {
	char* dst = malloc(len + 1);
	if(dst) {
		memmove(dst, str, len);
		dst[len] = '\0';
	}
	return dst;
}

KV* kvNew(const char* key, const char* value) {
	KV* kv = malloc(sizeof(KV));

	kv->key = strndup(key, strlen(key));
	kv->value = strndup(value, strlen(value));

	return kv;
}

void kvDel(KV* kv) {
	free(kv->key);
	free(kv->value);
	free(kv);
}

static bool kvDelEach(void* kv) {
	if(kv) {
		free(((KV*)kv)->key);
		free(((KV*)kv)->value);
	}

	return true;
}

void kvDelList(Node* list) {
	listForEach(list, kvDelEach);
	clear(list);
}

static bool kvPrintEach(void* kv) {
	fprintf(stdout, "%s: %s\n", ((KV*)kv)->key, ((KV*)kv)->value);

	return true;
}

void kvPrintList(Node* list) {
	listForEach(list, kvPrintEach);
}

char* kvFindList(const Node* head, const char* key) {
	Node* iter = (Node*)head;
	while(iter) {
		const KV* item = iter->value;
		if(!strcmp(item->key, key))
			return item->value;
		iter = iter->next;
	}

	return NULL;
}
