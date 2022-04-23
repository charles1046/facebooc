#include "http/cookies.h"
#include "http/helper.h"
#include <stdlib.h>
#include <string.h>

#define min(x, y) ((x < y) ? (x) : (y))

static inline const char* cookie_data(const Node* header) {
	static const char* ck_str = "cookie";
	const size_t ck_str_len = sizeof(ck_str) / sizeof(char);  // Compute it in compile time
	while(header) {
		const KV* entry = (KV*)header->value;
		const size_t entry_key_len = strlen(entry->key);
		if(!strncmp(entry->key, ck_str, min(entry_key_len, ck_str_len)))  // is "cookie"
			return entry->value;
		header = header->next;
	}

	return NULL;
}

Node* cookies_parser(const Node* header) {
	if(!header)
		return NULL;
	const char* str = cookie_data(header);
	if(!str)  // No cookie
		return NULL;

	// The str is begin with non-whitespace
	// cookie-header    = "Cookie:" OWS cookie-string OWS
	// cookie-string    = cookie-pair *(";" SP cookie-pair)
	Node* cookies = NULL;
	while(*str) {
		struct string_view sep = string_view_ctor(str, "; ");
		if(sep.size != -1) {
			CONST_INIT(*(sep.end - 1), (char)0);
		}

		// Check if it have '='
		if(strchr(str, '=')) {	// transfter url encoding
			char* decoded = url_decoder(str);
			KV* entry = query_entry(decoded);
			cookies = insert(entry, sizeof(*entry), cookies);
			free(decoded);
		}

		str += strlen(str) + 1;
	}

	return cookies;
}