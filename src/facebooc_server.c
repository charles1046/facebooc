#include "facebooc_server.h"
#include "db.h"
#include "http/cookies.h"
#include "server.h"
#include "template.h"
#include "utility.h"

// handlers of facebooc
#include "handler/handlers/connect.h"
#include "handler/handlers/dashboard.h"
#include "handler/handlers/home.h"
#include "handler/handlers/like.h"
#include "handler/handlers/login.h"
#include "handler/handlers/logout.h"
#include "handler/handlers/not_found.h"
#include "handler/handlers/post.h"
#include "handler/handlers/profile.h"
#include "handler/handlers/search.h"
#include "handler/handlers/signup.h"
#include "handler/handlers/static_handler.h"
#include "handler/handlers/unlike.h"

// Facebooc is a directive class of Server
struct Facebooc {
	Server* server;
};

static void FB_set_handlers(Facebooc* fb);

Facebooc* FB_new(const uint16_t port) {
	Facebooc* fb = malloc(sizeof(Facebooc));
	fb->server = serverNew(port);

	FB_set_handlers(fb);

	initDB();

	return fb;
}

static void FB_set_handlers(Facebooc* fb) {
	Server* server = fb->server;
	serverAddHandler(server, "signup", signup);
	serverAddHandler(server, "logout", logout);
	serverAddHandler(server, "login", login);
	serverAddHandler(server, "search", search);
	serverAddHandler(server, "connect", connect);
	serverAddHandler(server, "like", like);
	serverAddHandler(server, "unlike", unlike);
	serverAddHandler(server, "post", post);
	serverAddHandler(server, "profile", profile);
	serverAddHandler(server, "dashboard", dashboard);
	serverAddHandler(server, "", home);
	serverAddHandler(server, "static", static_handler);

	set_callback(server, not_found);
}

int FB_run(Facebooc* fb) {
	serverServe(fb->server);
	return 1;
}

void FB_delete(Facebooc* fb) {
	if(fb->server)
		serverDel(fb->server);
	db_close();
	free(fb);
	fb = NULL;
}
