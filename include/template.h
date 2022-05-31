#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "list.h"

typedef struct Template {
	char* filename;

	Node* context;
} Template;

Template* templateNew(char*);
void templateDel(Template*);
void templateSet(Template* template, const char* KEY, const char* VALUE);
char* templateRender(Template*);

#endif
