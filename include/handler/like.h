#ifndef __HANDLER_LIKE__
#define __HANDLER_LIKE__

#include "facebooc_server.h"

Response *like(const Request *req);
Response *unlike(const Request *req);

#endif
