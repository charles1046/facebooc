#include "http/helper.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void computeLPSArray(const char* pat, size_t M, int* lps);

size_t KMPSearch(const char* pat, const char* txt) {
	size_t M = strlen(pat);
	size_t N = strlen(txt);

	// create lps[] that will hold the longest prefix suffix
	// values for pattern
	int* lps = (int*)malloc(sizeof(int) * M);
	size_t j = 0;  // index for pat[]

	// Preprocess the pattern (calculate lps[] array)
	computeLPSArray(pat, M, lps);

	size_t i = 0;  // index for txt[]
	while(i < N) {
		if(pat[j] == txt[i]) {
			j++;
			i++;
		}

		if(j == M) {
			return i - j;
		}

		// mismatch after j matches
		else if(i < N && pat[j] != txt[i]) {
			// Do not match lps[0..lps[j-1]] characters,
			// they will match anyway
			if(j != 0)
				j = lps[j - 1];
			else
				i = i + 1;
		}
	}
	free(lps);
	return -1;
}

void computeLPSArray(const char* pat, size_t M, int* lps) {
	int len = 0;  // length of the previous longest prefix suffix
	lps[0] = 0;
	size_t i = 1;

	while(i < M) {
		if(pat[i] == pat[len]) {
			len++;
			lps[i] = len;
			i++;
		}
		else {
			if(len != 0) {
				len = lps[len - 1];
			}
			else {
				lps[i] = 0;
				i++;
			}
		}
	}
}

// https://stackoverflow.com/a/14530993
static void urldecode2(char* dst, const char* src) {
	char a, b;
	while(*src) {
		if((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
			if(a >= 'a')
				a -= 'a' - 'A';
			if(a >= 'A')
				a -= ('A' - 10);
			else
				a -= '0';
			if(b >= 'a')
				b -= 'a' - 'A';
			if(b >= 'A')
				b -= ('A' - 10);
			else
				b -= '0';
			*dst++ = 16 * a + b;
			src += 3;
		}
		else if(*src == '+') {
			*dst++ = ' ';
			src++;
		}
		else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
}

static inline char* string_view_dup(const struct string_view* const sv) {
	char* copy = NULL;
	if(sv->size != -1) {
		copy = malloc(sv->size + 1);
		strncpy(copy, sv->begin, sv->size);
		copy[sv->size] = 0;
	}
	else {
		copy = malloc(1);
		*copy = 0;
	}
	return copy;
}

void to_lower_case(char* str) {
	while(*str) {
		*str = tolower(*str);
		str++;
	}
}

struct string_view string_view_ctor(const char* buf, const char* delim) {
	struct string_view entry;
	entry.begin = buf;
	entry.end = find_first_of(buf, delim) + 1;
	CONST_INIT(entry.size, entry.end - entry.begin);

	if(entry.end == NULL) {
		CONST_INIT(entry.size, -1);
	}
	return entry;
}

// KMP
// https://riptutorial.com/algorithm/example/23880/kmp-algorithm-in-c
char* find_first_of(const char* str, const char* delim) {
	size_t pos = KMPSearch(str, delim);
	if(pos)
		return (char*)(str + pos);
	else
		return NULL;
}

// Support unicode
char* url_decoder(const char* str) {
	// Each '%' symbol will brought another 2 characters behind it
	size_t percent_counter = 0;
	char* shadow_str = (char*)str;
	while(*shadow_str++)
		++percent_counter;

	char* new_url = malloc(strlen(str) - percent_counter * 2);

	urldecode2(new_url, str);
	return new_url;
}

KV* make_pair(const struct string_view* const key, const struct string_view* const value) {
	KV* entry = malloc(sizeof(KV));
	entry->key = string_view_dup(key);
	entry->value = string_view_dup(value);

	return entry;
}
