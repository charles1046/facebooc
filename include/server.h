#ifndef SERVER_H
#define SERVER_H

#include "hash_map.h"
#include "request.h"
#include "response.h"
#include <stdint.h>
typedef struct Server Server;
typedef Response* (*Handler)(Request*);

struct Server {
	unsigned int port;
	uintptr_t priv;
	Hash_map* handlers;
	Handler default_callback;
};

Server* serverNew(uint16_t);
void serverAddHandler(Server* server, const char* route_name, Handler handler);
void set_callback(Server* server, Handler handler);
void serverDel(Server* server);
void serverServe(Server* server);

#endif
