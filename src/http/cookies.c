#include "http/cookies.h"
#include "hash_map.h"
#include "http/helper.h"
#include "utility.h"
#include <stdlib.h>
#include <string.h>

#define min(x, y) ((x < y) ? (x) : (y))

struct Cookie {
	Hash_map* dict;
};

Cookie* Cookie_new(void) {
	Cookie* c = malloc(sizeof(Cookie));
	c->dict = Hash_map_new();
	return c;
}

Cookie* Cookie_insert(Cookie* c, const SPair* p) {
	if(unlikely(!c || !p))
		return c;

	(void)Hash_map_insert(c->dict, p);
	return c;
}

Cookie* Cookie_insert_move(Cookie* c, SPair* p) {
	if(unlikely(!c || !p))
		return c;

	(void)Hash_map_insert_move(c->dict, p);
	return c;
}

char* Cookie_get(const Cookie* c, const char* key) {
	if(unlikely(!c || !key))
		return NULL;

	return Hash_map_get(c->dict, key);
}

void Cookie_delete(Cookie* c) {
	Hash_map_delete(c->dict);
	free(c);
	c = NULL;
}

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

Cookie* cookies_parser(const Node* header) {
	if(!header)
		return NULL;
	const char* str = cookie_data(header);
	if(!str)  // No cookie
		return NULL;

	// The str is begin with non-whitespace
	// cookie-header    = "Cookie:" OWS cookie-string OWS
	// cookie-string    = cookie-pair *(";" SP cookie-pair)
	Cookie* cookies = Cookie_new();
	while(*str) {
		struct string_view sep = string_view_ctor(str, "; ");
		if(sep.size != -1) {
			CONST_INIT(*(sep.end - 1), (char)0);
		}

		// Check if it have '='
		if(strchr(str, '=')) {	// transfer url encoding
			char* decoded = url_decoder(str);
			// TODO: Remove the legacy KV struct
			KV* entry = query_entry(decoded);
			Hash_map_insert_move(cookies->dict, (SPair*)entry);
			free(decoded);
		}
		str += strlen(str) + 1;
	}

	return cookies;
}