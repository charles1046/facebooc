#ifndef REQUEST_H
#define REQUEST_H

#include <string.h>

#include "list.h"

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
} Request;

Request* requestNew(char*);
void requestDel(Request*);

#endif
