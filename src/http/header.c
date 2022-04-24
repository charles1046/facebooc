#include "http/header.h"
#include "http/helper.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static inline void strip_head_spaces(struct string_view* sv) {
	while(isspace(*sv->begin) && sv->begin != sv->end) {
		sv->begin++;
		CONST_INIT(sv->size, sv->size - 1);
	}

	if(sv->begin == sv->end)
		CONST_INIT(sv->size, -1L);
}

// RFC2616 section 4.2
Node* header_parser(char* buf, size_t* offset) {
	Node* header = NULL;
	char* base = buf;

	while(*buf) {  // Read all until the end
		// Read line end with CRLF
		struct string_view line = string_view_ctor(buf, "\r\n");

		if(line.size == 0)	// empty line, that is the end of header
			break;

		const char* seperator = find_first_of(line.begin, ":");
		assert(seperator);

		struct string_view key = { .begin = line.begin,
								   .end = seperator,
								   .size = seperator - line.begin };
		struct string_view value = { .begin = seperator + 1,
									 .end = line.end,
									 .size = line.end - (seperator + 1) };
		CONST_INIT(*key.end, (char)0);	  // Force set ':' to '\0'
		CONST_INIT(*value.end, (char)0);  // Force set '\r' to '\0'

		// key is case-insensitive
		to_lower_case((char*)key.begin);
		strip_head_spaces(&value);

		KV* entry = make_pair(&key, &value);
		header = insert(entry, sizeof(*entry), header);

		buf += line.size + 2;  // Skip '\r' and '\n'
	}

	// Return offset to the caller(aka. request)
	// The request_line already have some value
	*offset += buf + 2 - base;

	return header;
}

// char* header_generator(const Node* const header_list) {}