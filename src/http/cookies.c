#include "http/cookies.h"
#include "hash_map.h"
#include "http/helper.h"
#include "pair.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define min(x, y) ((x < y) ? (x) : (y))

// rfc6265, Section 4.1.1
// set-cookie-string  = cookie-pair *( ";" SP cookie-av )
// 		   cookie-av  = expires-av / max-age-av / domain-av /
//  					path-av / secure-av / httponly-av /
//  					extension-av
struct Cookie_av {
	const char* expires;
	const char* max_age;
	const char* domain;
	const char* path;
	int secure;	   // True or False
	int httponly;  // True or False
};

struct Cookie_av_to_string_template__ {
	const char* expires;
	const char* max_age;
	const char* domain;
	const char* path;
	const char* secure;
	const char* httponly;
};

struct Cookies {
	Hash_map* dict;
};

Cookies* Cookie_new(void) {
	Cookies* c = malloc(sizeof(Cookies));
	c->dict = Hash_map_new();
	return c;
}

Cookies* Cookie_insert(Cookies* c, const SPair* p) {
	if(unlikely(!c || !p))
		return c;

	(void)Hash_map_insert(c->dict, p);
	return c;
}

Cookies* Cookie_insert_move(Cookies* c, SPair* p) {
	if(unlikely(!c || !p))
		return c;

	(void)Hash_map_insert_move(c->dict, p);
	return c;
}

char* Cookie_get(const Cookies* c, const char* key) {
	if(unlikely(!c || !key))
		return NULL;

	return Hash_map_get(c->dict, key);
}

void Cookie_delete(Cookies* c) {
	Hash_map_delete(c->dict);
	free(c);
	c = NULL;
}

Cookies* cookies_parser(const Header* header) {
	if(!header)
		return NULL;
	const char* str = header_get(header, "cookie");
	if(!str)  // No cookie
		return NULL;

	// The str is begin with non-whitespace
	// cookie-header    = "Cookie:" OWS cookie-string OWS
	// cookie-string    = cookie-pair *(";" SP cookie-pair)
	Cookies* cookies = Cookie_new();
	while(*str) {
		struct string_view sep = string_view_ctor(str, "; ");
		if(sep.size != -1) {
			CONST_INIT(*(sep.end - 1), (char)0);
		}

		// Check if it have '='
		if(strchr(str, '=')) {	// transfer url encoding
			char* decoded = url_decoder(str);
			SPair* entry = query_entry(decoded);
			Hash_map_insert_move(cookies->dict, entry);
			free(decoded);
		}
		str += strlen(str) + 1;
	}

	return cookies;
}

Cookie_av* cookie_av_new() {
	Cookie_av* c_av = malloc(sizeof(Cookie_av));
	c_av->expires = NULL;
	c_av->domain = NULL;
	c_av->path = NULL;
	c_av->max_age = NULL;
	c_av->secure = 0;
	c_av->httponly = 0;
	return c_av;
}

// The c_av must not be a nullptr
static bool c_av_is_empty(const Cookie_av* c_av) {

	return !c_av->expires && !c_av->domain && !c_av->path && !c_av->max_age && !c_av->httponly
		   && !c_av->secure;
}

static inline size_t cav_template_len(const struct Cookie_av_to_string_template__* t) {
	return strlen(t->expires) + strlen(t->max_age) + strlen(t->domain) + strlen(t->path)
		   + strlen(t->secure) + strlen(t->httponly);
}

const char* concatenate_cookie_av(SSPair* restrict cookie, const Cookie_av* restrict c_av) {
	if(unlikely(!cookie || !cookie->key || !cookie->value || !c_av || c_av_is_empty(c_av)))
		return NULL;

	//  set-cookie-string = cookie-pair *( ";" SP cookie-av )
	size_t total_len = strlen(cookie->key) + strlen(cookie->value) + 1;	 // Add the '='
	const struct Cookie_av_to_string_template__ t = {
		.expires = c_av->expires ? c_av->expires : "",
		.max_age = c_av->max_age ? c_av->max_age : "",
		.domain = c_av->domain ? c_av->domain : "",
		.path = c_av->path ? c_av->path : "/",	// Defalut path is root
		.secure = c_av->secure ? "; Secure" : "",
		.httponly = c_av->httponly ? "; HttpOnly" : "",
	};
	total_len += cav_template_len(&t);

	char* new_val = malloc(total_len);
	snprintf(new_val, total_len, "%s=%s%s%s%s%s%s%s", cookie->key, cookie->value, t.expires,
			 t.max_age, t.domain, t.path, t.secure, t.httponly);

	// Replace old value which is mappped
	free(cookie->value);
	cookie->value = new_val;
	return new_val;
}

// RFC 1123
//  expires-av        = "Expires=" sane-cookie-date
//  sane-cookie-date  = <rfc1123-date, defined in [RFC2616], Section 3.3.1>
void cookie_av_set_expires(Cookie_av* c_av, int duration) {
	if(unlikely(!c_av))
		return;

	free((char*)c_av->expires);

	const time_t t = time(NULL) + duration;
	char sbuff[100];
	strftime(sbuff, 100, "; Expires=%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

	c_av->expires = strdup(sbuff);
}

void cookie_av_set_maxage(Cookie_av* c_av, int age) {
	if(unlikely(!c_av))
		return;

	free((char*)c_av->max_age);

	char* str = strdup("; Max-Age=0");
	str[10] += age;
	c_av->max_age = str;
}

void cookie_av_set_domain(Cookie_av* restrict c_av, const char* restrict domain) {
	if(unlikely(!c_av || !domain))
		return;

	free((char*)c_av->domain);

	size_t len = strlen(domain);
	char* domain_str = malloc(len + 12);
	snprintf(domain_str, 12 + len, "; Domain=%s", domain);
	c_av->domain = domain_str;
}

void cookie_av_set_path(Cookie_av* restrict c_av, const char* restrict path) {
	if(unlikely(!c_av || !path))
		return;

	free((char*)c_av->path);

	size_t len = strlen(path);
	char* path_str = malloc(len + 7);
	snprintf(path_str, 7 + len, "; Path=%s", path);
	c_av->path = path_str;
}

void cookie_av_set_secure(Cookie_av* c_av, int is_secure) {
	if(unlikely(!c_av))
		return;

	c_av->secure = is_secure;
}

void cookie_av_set_httponly(Cookie_av* c_av, int is_httponly) {
	if(unlikely(!c_av))
		return;

	c_av->secure = is_httponly;
}

void cookie_av_delete(Cookie_av* c_av) {
	if(!c_av)
		return;

	free((char*)c_av->expires);
	free((char*)c_av->path);
	free((char*)c_av->domain);
	free((char*)c_av->max_age);
	c_av->expires = NULL;
	c_av->domain = NULL;
	c_av->path = NULL;
	c_av->max_age = NULL;
}