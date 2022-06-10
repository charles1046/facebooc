#include "handler/handler_utility.h"

#include <stdlib.h>
#include <string.h>

int get_id(const char* uri) {
	// format: "/%s/<id>/"

	char* begin = strchr(uri + 1, '/') + 1;
	char* end = strchr(begin, '/');
	const size_t len = end - begin;

	char new_str[10] = { 0 };
	memcpy(new_str, begin, len);

	return atoi(new_str);
}

void template_set_error_message(Template* template, const char* error_type,
								const char* error_message) {
	templateSet(template, error_type, error_message);
}
