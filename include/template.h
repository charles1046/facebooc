#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "basic_string.h"
#include "list.h"

typedef struct Template Template;

Template* templateNew(const char* filename);
void templateDel(Template* t);

// Would overwrite the origin key-value pair
void templateSet(Template* template, const char* key, const char* value);
Basic_string* templateRender(const Template* t);

#endif
