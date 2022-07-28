#ifndef __BASE64_H__
#define __BASE64_H__
#include "basic_string.h"

Basic_string* base64_encode(const void* data, size_t len);

// Return: data in dst, which len is stored in dst_len
void base64_decode(void** dst, size_t* dst_len, const Basic_string* src);

// Suppose decoded value is a string
Basic_string* base64_decode2str(const Basic_string* src);

#define base64_decode_at(dst_name, src) \
	void* dst_name = NULL;              \
	size_t dst_name##_len;              \
	base64_decode(&dst_name, &dst_name##_len, (src))

#endif