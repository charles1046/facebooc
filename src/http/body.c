#include "http/body.h"
#include "http/helper.h"
#include "http/query.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

// If you want to implement Content-Type please refer RFC 2616 7.1

static inline _Bool is_sepported_type(const char* str) {
	if(!str)
		false;
	return !strncmp(str, "application/x-www-form-urlencoded", 33);
}

static inline _Bool is_content_type(const char* str) {
	if(!str)
		false;
	return !strncmp(str, "content-type", 13);
}

static inline _Bool check_header(const Node* header) {
	while(header) {
		const KV* item = header->value;
		if(is_content_type(item->key) && is_sepported_type(item->value))
			return true;
		header = header->next;
	}
	return false;
}

Node* body_parser(const Node* header, char* buffer) {
	if(!check_header(header) || !buffer)
		return NULL;

	return query_parser(buffer);
}