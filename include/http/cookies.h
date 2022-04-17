#ifndef __COOKIES_H__
#define __COOKIES_H__

#include "list.h"
#include <stdbool.h>

// Refers to RFC 6265
Node* cookies_parser(const Node* header);

#endif