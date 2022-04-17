#include "http/cookies.h"
#include "http/helper.h"
#include <string.h>

Node* cookies_parser(const Node* header) {
	if(!header)
		return NULL;

	Node* cookies = NULL;

	// TODO: Parsing the cookie-string
	// The str is begin with non-whitespace
	// cookie-header    = "Cookie:" OWS cookie-string OWS
	// cookie-string    = cookie-pair *(";" SP cookie-pair)

	return cookies;
}