#ifndef __BODY_HTTP_H__
#define __BODY_HTTP_H__

#include "list.h"
#include <stdbool.h>

Node* body_parser(const Node* header, char* buffer);
// TODO: body generator

#endif