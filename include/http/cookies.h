#ifndef __COOKIES_H__
#define __COOKIES_H__

#include "http/header.h"
#include "pair.h"
#include <stdbool.h>

// All cookie's element is
// (const char *)(key):(char *)(value)
// A.K.A const string maps to string
typedef struct Cookies Cookies;
typedef struct Cookie_av Cookie_av;

// Refers to RFC 6265
// Return a new instance of Cookies structure, you should free it via Cookie_delete
Cookies* cookies_parser(const Header* header);

Cookies* Cookie_new(void);
// Add a string-void pair to the cookie
// Return the Cookies c
// Will "copy" the p into the Cookies c
Cookies* Cookie_insert(Cookies* c, const SPair* p);
// Move semantics imple.
Cookies* Cookie_insert_move(Cookies* c, SPair* p);
// Use key to search the value in cookie
// Return NULL if not found
char* Cookie_get(const Cookies* c, const char* key);
void Cookie_delete(Cookies*);

// For response setting attributes
// Return concatenated value, the origin value will be also replaced
const char* concatenate_cookie_av(SSPair* restrict cookie, const Cookie_av* restrict c_av);

Cookie_av* cookie_av_new();
void cookie_av_set_expires(Cookie_av* restrict c_av, int duration);
// age must be [1, 9]
void cookie_av_set_maxage(Cookie_av* c_av, int age);
void cookie_av_set_domain(Cookie_av* restrict c_av, const char* restrict domain);
void cookie_av_set_path(Cookie_av* restrict c_av, const char* restrict path);
void cookie_av_set_secure(Cookie_av* c_av, int is_secure);
void cookie_av_set_httponly(Cookie_av* c_av, int is_httponly);
void cookie_av_delete(Cookie_av*);

#endif