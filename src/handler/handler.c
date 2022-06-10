#include "handler/handler.h"

#include <string.h>

#include "hash_map.h"
#include "server.h"
#include "utility.h"

// default callback function
#include "handler/handlers/not_found.h"

// static file handler
#include "handler/handlers/static_handler.h"

// handlers
#include "handler/handlers/connect.h"
#include "handler/handlers/dashboard.h"
#include "handler/handlers/home.h"
#include "handler/handlers/like.h"
#include "handler/handlers/login.h"
#include "handler/handlers/logout.h"
#include "handler/handlers/post.h"
#include "handler/handlers/profile.h"
#include "handler/handlers/search.h"
#include "handler/handlers/signup.h"
#include "handler/handlers/unlike.h"

static inline void add_handler(Hash_map* handlers, char* handling_name, Handler handler) {
	SPair* new_handler = malloc(sizeof(SPair));
	new_handler->key = strdup(handling_name);
	new_handler->value = memdup(&handler, sizeof(Handler));
	Hash_map_insert_move(handlers, new_handler);
}

static inline char* get_handling_type(Request* request) {
	char* handling_type = strdup(request->uri + 1);
	char* last_slash = strrchr(handling_type, '/');
	if(last_slash)
		*last_slash = '\0';
	return handling_type;
}

static inline void add_callback_handler(Hash_map* handlers) {
	add_handler(handlers, "not_found", not_found);
}
void server_add_handlers(Server* server) {
	Hash_map* handlers = server->handlers;
	add_handler(handlers, "signup", signup);
	add_handler(handlers, "logout", logout);
	add_handler(handlers, "login", login);
	add_handler(handlers, "search", search);
	add_handler(handlers, "connect", connect);
	add_handler(handlers, "like", like);
	add_handler(handlers, "unlike", unlike);
	add_handler(handlers, "post", post);
	add_handler(handlers, "profile", profile);
	add_handler(handlers, "dashboard", dashboard);
	add_handler(handlers, "", home);
	add_handler(handlers, "static", static_handler);

	add_callback_handler(handlers);
}

Response* handle_request(Hash_map* handlers, Request* request) {
	char* handling_type = get_handling_type(request);

	Handler* handler = Hash_map_get(handlers, handling_type);
	if(handler == NULL)	 // not found
		handler = Hash_map_get(handlers, "not_found");

	Response* response = (*handler)(request);

	return response;
}