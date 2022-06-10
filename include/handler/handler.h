#ifndef HANDLER_H
#define HANDLER_H

#include "hash_map.h"
#include "request.h"
#include "response.h"
#include "server.h"

// Add handlers to server
void server_add_handlers(Server*);

// Handling request
Response* handle_request(Hash_map* handlers, Request* request);

#endif