#include "facebooc_server.h"
#include "db.h"
#include "http/cookies.h"
#include "server.h"
#include "template.h"
#include "utility.h"

#include "handler/handler.h"

// Facebooc is a directive class of Server
struct Facebooc {
	Server* server;
};

Facebooc* FB_new(const uint16_t port) {
	Facebooc* s = malloc(sizeof(Facebooc));
	s->server = serverNew(port);

	server_add_handlers(s->server);

	initDB();

	return s;
}

int FB_run(Facebooc* s) {
	serverServe(s->server);
	return 1;
}

void FB_delete(Facebooc* s) {
	if(s->server)
		serverDel(s->server);
	db_close();
	free(s);
	s = NULL;
}
