#ifndef __HEADER_HTTP_H__
#define __HEADER_HTTP_H__

#include "list.h"
#include <stdbool.h>

Node* header_parser(char* buffer, size_t* offset);
char* header_generator(const Node* const header_list);

#endif