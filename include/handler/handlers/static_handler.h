#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include "hash_map.h"
#include "request.h"
#include "response.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

typedef struct buf_size BufSize;

Response* static_handler(Request* req);

#endif