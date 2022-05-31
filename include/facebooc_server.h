#ifndef __FACEBOOC_H__
#define __FACEBOOC_H__

#include "server.h"

typedef struct Facebooc Facebooc;

Facebooc* FB_new(uint16_t port);
int FB_run(Facebooc* s);
void FB_delete(Facebooc* s);

// You should add supported routings in facebooc_server.c

#endif