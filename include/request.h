#ifndef REQUEST_H
#define REQUEST_H

#include <string.h>

#include "list.h"
#include "models/account.h"

typedef enum Method {
	OPTIONS,
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	TRACE,
	CONNECT,
	UNKNOWN_METHOD
} Method;

typedef struct Request {
	const Method method;

	const char* path;
	const char* uri;

	const Node* queryString;
	const Node* postBody;
	const Node* cookies;
	const Node* headers;

	Account* account;  // Don't worry. You're doing fine. Game is hard.
} Request;

Request* requestNew(char*);
void requestDel(Request*);

#define EXACT_ROUTE(req, routeString)                        \
	{                                                        \
		const char* route = routeString "\0";                \
		if(strncmp(req->uri, route, strlen(route) + 1) != 0) \
			return NULL;                                     \
	}

#define ROUTE(req, routeString)                          \
	{                                                    \
		const char* route = routeString;                 \
		if(strncmp(req->uri, route, strlen(route)) != 0) \
			return NULL;                                 \
	}

#endif
