#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "list.h"

typedef struct Template {
	const char* filename;

	Node* context;
} Template;

Template* templateNew(const char*);
void templateDel(Template*);
void templateSet(Template*, const char* key, const char* value);
char* templateRender(Template*);

#endif
