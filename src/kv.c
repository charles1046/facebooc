#include <stdio.h>
#include <string.h>

#include "bs.h"
#include "kv.h"

KV* kvNew(char* key, char* value) {
	KV* kv = malloc(sizeof(KV));

	kv->key = bsNew(key);
	kv->value = bsNew(value);

	return kv;
}

void kvDel(KV* kv) {
	bsDel(kv->key);
	bsDel(kv->value);
	free(kv);
}

static bool kvDelEach(void* kv) {
	if(kv) {
		bsDel(((KV*)kv)->key);
		bsDel(((KV*)kv)->value);
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
