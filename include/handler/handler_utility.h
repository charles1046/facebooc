#ifndef HANDLER_UTILITIES_H
#define HANDLER_UTILITIES_H

#include "template.h"

int get_id(const char*);

void template_set_error_message(Template* template_, const char* error_type,
								const char* error_message);

#endif