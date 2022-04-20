#include "http/query.h"
#include <http/helper.h>
#include <string.h>

// Case sensitive
static inline KV* new_entry(const char* str) {
	const char* seperator = find_first_of(str, "=");
	struct string_view key = { .begin = str, .end = seperator, .size = seperator - str };
	struct string_view value = { .begin = seperator + 1,
								 .end = str + strlen(str) + 1,
								 .size = strlen(str) - key.size - 1 };
	return make_pair(&key, &value);
}

Node* query_parser(char* path) {
	Node* queries = NULL;
	const char* bound = path + strlen(path) + 1;

	char* start = find_first_of(path, "?");
	do {
		struct string_view entry = string_view_ctor(start, "&");
		if(entry.size == -1)  // is the last query
			start[strlen(start) + 1] = '\0';
		else
			CONST_INIT(*entry.end, 0);	// Force set the '&' to '0'
		KV* item = new_entry(start);
		queries = insert(item, sizeof(item), queries);

		start += strlen(start) + 2;	 // Skip the '\0'
	} while(start && start < bound);

	return queries;
}