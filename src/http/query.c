#include "http/query.h"
#include <assert.h>
#include <http/helper.h>
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

Node* query_parser(char* buffer) {
	Node* list = NULL;
	while(*buffer) {
		struct string_view sep = string_view_ctor(buffer, "&");
		if(sep.size != -1) {
			CONST_INIT(*(sep.end - 1), (char)0);
		}

		// Check if it have '='
		if(strchr(buffer, '=')) {  // transfter url encoding
			char* decoded = url_decoder(buffer);
			KV* entry = query_entry(decoded);
			list = insert(entry, sizeof(*entry), list);
			free(decoded);
		}

		buffer += strlen(buffer) + 1;
	}

	return list;
}