#ifndef __HELPER_HTTP_H__
#define __HELPER_HTTP_H__

#include "kv.h"
#include <stddef.h>
// Using __typeof__ extension by GCC
#define CONST_INIT(x, value) *(__typeof__(value)*)(&x) = value

// Simulate C++'s string_view
struct string_view {
	const char* begin;
	const char* end;
	const long size;
};

struct string_view string_view_ctor(const char* buf, const char* delim);

void to_lower_case(char* str);
char* find_first_of(const char* str, const char* delim);
char* url_decoder(const char* str);
KV* make_pair(const struct string_view* const key, const struct string_view* const value);

#endif