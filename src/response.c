#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "bs.h"
#include "kv.h"
#include "response.h"

const char* STATUSES[5][25] = {
	{ "Continue", "Switching Protocols" },
	{ "OK", "Created", "Accepted", "Non-Authoritative Information", "No Content", "Reset Content",
	  "Partial Content" },
	{ "Multiple Choices", "Moved Permanently", "Found", "See Other", "Not Modified", "Use Proxy",
	  "Switch Proxy", "Temporary Redirect", "Permanent Redirect" },
	{ "Bad Request",
	  "Unauthorized",
	  "Payment Required",
	  "Forbidden",
	  "Not Found",
	  "Method Not Allowed",
	  "Not Acceptable",
	  "Proxy Authentication Required",
	  "Request Timeout",
	  "Conflict",
	  "Gone",
	  "Length Required",
	  "Precondition Failed",
	  "Request Entity Too Large",
	  "Request-URI Too Long",
	  "Unsupported Media Type",
	  "Requested Range Not Satisfiable",
	  "Expectation Failed",
	  "I'm A Teapot",
	  "Authentication Timeout",
	  "Enhance Your Calm" },
	{ "Internal Server Error", "Not Implemented", "Bad Gateway", "Service Unavailable",
	  "Gateway Timeout", "HTTP Version Not Supported" },
};

Response* responseNew() {
	Response* response = malloc(sizeof(Response));
	response->status = OK;
	response->headers = NULL;
	response->body = NULL;
	return response;
}

Response* responseNewRedirect(char* location) {
	Response* response = responseNew();

	responseSetStatus(response, FOUND);
	responseAddHeader(response, "Location", location);

	return response;
}

void responseSetStatus(Response* response, Status status) {
	response->status = status;
}

void responseSetBody(Response* response, char* body) {
	response->body = body;
}

void responseAddCookie(Response* response, char* key, char* value, char* domain, char* path,
					   int duration) {
	char cbuff[512];
	char sbuff[100];
	time_t t = time(NULL) + duration;

	strftime(sbuff, 100, "%c GMT", gmtime(&t));

	const size_t key_len = strlen(key);
	const size_t val_len = strlen(value);
	const size_t sbuff_len = strlen(sbuff);

	snprintf(cbuff, 12 + key_len + val_len + sbuff_len, "%s=%s; Expires=%s", key, value, sbuff);

	if(domain) {
		const size_t domain_len = strlen(domain);
		snprintf(sbuff, domain_len + 10, "; Domain=%s", domain);
		strcat(cbuff, sbuff);
	}

	if(path) {
		const size_t path_len = strlen(path);
		snprintf(sbuff, 8 + path_len, "; Path=%s", path);
		strcat(cbuff, sbuff);
	}
	else {
		strcat(cbuff, "; Path=/");
	}

	responseAddHeader(response, "Set-Cookie", cbuff);
}

void responseAddHeader(Response* response, char* key, char* value) {
	response->headers = insert(kvNew(key, value), sizeof(KV), response->headers);
}

void responseDel(Response* response) {
	if(response->headers)
		kvDelList(response->headers);
	if(response->body)
		bsDel(response->body);

	free(response);
}

void responseWrite(Response* response, int fd) {
	Node* buffer = NULL;
	Node* header;

	char sbuffer[2048];

	// HEADERS
	header = response->headers;

	while(header) {
		const size_t key_len = strlen(((KV*)header->value)->key);
		const size_t val_len = strlen(((KV*)header->value)->value);
		snprintf(sbuffer, 5 + key_len + val_len, "%s: %s\r\n", ((KV*)header->value)->key,
				 ((KV*)header->value)->value);

		buffer = insert(sbuffer, sizeof(char) * (strlen(sbuffer) + 1), buffer);
		header = (Node*)header->next;
	}

	// STATUS
	const size_t status_len = strlen(STATUSES[response->status / 100 - 1][response->status % 100]);
	snprintf(sbuffer, 13 + 10 + status_len, "HTTP/1.0 %d %s\r\n", response->status,
			 STATUSES[response->status / 100 - 1][response->status % 100]);

	buffer = insert(sbuffer, sizeof(char) * (strlen(sbuffer) + 1), buffer);

	// OUTPUT
	while(buffer) {
		send(fd, buffer->value, strlen(buffer->value), 0);

		buffer = (Node*)buffer->next;
	}

	send(fd, "\r\n", 2, 0);

	if(response->body)
		send(fd, response->body, bsGetLen(response->body), 0);
}
