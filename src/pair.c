#include "pair.h"
#include <string.h>

#define ARR_LEN(arr) (sizeof(arr) / sizeof(*arr))
#define ADD_ARR(type) #type

static const char* is_string[] = {
	ADD_ARR(char*),
	ADD_ARR(const char*),
	ADD_ARR(char const*),
};

static int subtract(const void* a, const void* b) {
	return *((const long*)a) - *((const long*)b);
}

CMP type_cmp(const char* type) {
	for(size_t i = 0; i < ARR_LEN(is_string); i++)
		if(strcmp(type, is_string[i]) == 0)
			return (CMP)strcmp;

	return subtract;
}