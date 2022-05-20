#include "utility.h"

// Reference: http://www.cse.yorku.ca/~oz/hash.html
int string_hash(const char* str) {
	int hash = 5381;
	int c;

	while(c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

// sdbm
int obj_hash(const void* data, size_t size) {
	unsigned long hash = 0;

	for(size_t i = 0; i < size; i++) {
		hash = ((int*)data)[i] + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}