#include "http/body.h"
#include "http/helper.h"
#include <stdlib.h>
#include <string.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))
// Case sensitive
static inline KV* new_entry(const char* str) {
	const char* seperator = find_first_of(str, "=");
	struct string_view key = { .begin = str, .end = seperator, .size = seperator - str };
	struct string_view value = { .begin = seperator + 1,
								 .end = str + strlen(str) + 1,
								 .size = strlen(str) - key.size - 1 };
	return make_pair(&key, &value);
}

// If you want to implement Content-Type please refer RFC 2616 7.1

static inline _Bool is_sepported_type(const char* str) {
	return !strncmp(str, "application/x-www-form-urlencoded", 33);
}

static inline _Bool is_content_type(const char* str) {
	return !strncmp(str, "content-type", 13);
}

static inline _Bool check_header(const Node* header) {
	while(header) {
		const KV* item = header->value;
		if(is_content_type(item->key) && is_sepported_type(item->value))
			return true;
	}
	return false;
}

Node* body_parser(const Node* header, char* buffer) {
	if(check_header(header) == false || !buffer)
		return NULL;

	// Buffer might be an empty string ""
	Node* bodies = NULL;
	while(*buffer) {
		struct string_view entry = string_view_ctor(buffer, "&");
		if(entry.size == -1)  // is the last query
			buffer[strlen(buffer) + 1] = '\0';
		else
			CONST_INIT(*entry.end, 0);	// Force set the '&' to '0'

		KV* item = new_entry(buffer);
		bodies = insert(item, sizeof(item), bodies);

		buffer += strlen(buffer) + 2;  // Skip the '\0'
	}

	return bodies;
}