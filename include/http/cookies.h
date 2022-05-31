#ifndef __COOKIES_H__
#define __COOKIES_H__

#include "list.h"
#include "pair.h"
#include <stdbool.h>

// All cookie's element is
// (const char *)(key):(char *)(value)
// A.K.A const string maps to string
typedef struct Cookie Cookie;

// Refers to RFC 6265
// Return a new instance of Cookie structure, you should free it via Cookie_delete
Cookie* cookies_parser(const Node* header);

Cookie* Cookie_new(void);
// Add a string-void pair to the cookie
// Return the Cookie c
// Will "copy" the p into the Cookie c
Cookie* Cookie_insert(Cookie* c, const SPair* p);
// Move semantics imple.
Cookie* Cookie_insert_move(Cookie* c, SPair* p);
// Use key to search the value in cookie
// Return NULL if not found
char* Cookie_get(const Cookie* c, const char* key);
void Cookie_delete(Cookie*);

#endif