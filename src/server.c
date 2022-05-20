#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
typedef int sockopt_t;

#include "bs.h"
#include "server.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

static char* METHODS[8] = { "OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT" };

static const int EPOLL_NUM = 64;

static inline void pop_log(const struct sockaddr_in* addr, const char* method, const char* path,
						   int status) {
	time_t t = time(NULL);
	char timebuff[100];
	strftime(timebuff, sizeof(timebuff), "%c", localtime(&t));
	fprintf(stdout, "%s %s %s %s %d\n", timebuff, inet_ntoa(addr->sin_addr), method, path, status);
}

Server* serverNew(uint16_t port) {
	Server* server = malloc(sizeof(Server));
	server->port = port;
	server->priv = epoll_create1(0);
	server->handlers = NULL;
	return server;
}

void serverDel(Server* server) {
	if(server->handlers)
		clear(server->handlers);
	free(server);
}

void serverAddHandler(Server* server, Handler handler) {
	HandlerP handlerP = &handler;
	server->handlers = insert(handlerP, sizeof(HandlerP), server->handlers);
}

// compare str1 and str2 from tail
static inline int rev_cmp(const char* str1, size_t len1, const char* str2, size_t len2) {
	const long smaller = min(len1, len2);
	return strncmp(str1 + len1 - smaller, str2 + len2 - smaller, smaller);
}

static inline const char* mimeType_mux(const char* path) {
	static const char* file_type[] = {
		"html", "json", "jpeg", "jpg", "gif", "png", "css", "js",
	};
	static const size_t file_type_len = sizeof(file_type) / sizeof(*file_type);

	static const char* mime_type[] = {
		"text/html", "application/json",	   "image/jpeg", "image/jpeg", "image/gif", "image/png",
		"text/css",	 "application/javascript", "text/plain",
	};

	size_t i = 0;
	for(; i < file_type_len; i++) {
		if(rev_cmp(path, strlen(path), file_type[i], strlen(file_type[i])) == 0)
			break;
	}

	return mime_type[i];
}

static Response* staticHandler(Request* req) {
	// Check is begin with "/static/"
	ROUTE(req, "/static/");

	// EXIT ON SHENANIGANS
	if(strstr(req->uri, "../"))
		return NULL;

	const char* filename = req->uri + 1;

	// EXIT ON DIRS
	struct stat sbuff;
	if(stat(filename, &sbuff) < 0 || S_ISDIR(sbuff.st_mode))
		return NULL;

	// EXIT ON NOT FOUND
	FILE* file = fopen(filename, "r");
	if(!file)
		return NULL;

	// GET LENGTH
	char lens[25];
	fseek(file, 0, SEEK_END);
	size_t len = ftell(file);
	snprintf(lens, 10, "%ld", (long int)len);
	rewind(file);

	// SET BODY
	Response* response = responseNew();

	char* buff = malloc(sizeof(char) * len);
	(void)!fread(buff, sizeof(char), len, file);
	responseSetBody(response, bsNewLen(buff, len));
	fclose(file);
	free(buff);

	// MIME TYPE
	const char* mimeType = mimeType_mux(req->uri);

	// RESPOND
	responseSetStatus(response, OK);
	responseAddHeader(response, "Content-Type", mimeType);
	responseAddHeader(response, "Content-Length", lens);
	responseAddHeader(response, "Cache-Control", "max-age=2592000");
	return response;
}

void serverAddStaticHandler(Server* server) {
	serverAddHandler(server, staticHandler);
}

static inline int makeSocket(unsigned int port) {
	const int sock = socket(PF_INET, SOCK_STREAM, 0);

	if(sock < 0) {
		fprintf(stderr, "error: failed to create socket\n");
		exit(1);
	}

	sockopt_t optval = 1; /* prevent from address being taken */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	const struct sockaddr_in addr = { .sin_family = AF_INET,
									  .sin_port = htons(port),
									  .sin_addr.s_addr = htons(INADDR_ANY) };

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "error: failed to bind socket to 0.0.0.0:%d\n", port);
		exit(1);
	}

	if(listen(sock, 1) < 0) {
		fprintf(stderr, "error: socket failed to listen\n");
		exit(1);
	}

	return sock;
}

static void setFdNonblocking(int fd) {
	if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0)
		perror("fcntl");
}

static void serverAddFd(int epollfd, int fd, int in, _Bool oneshot) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLET | EPOLLRDHUP;
	event.events |= in ? EPOLLIN : EPOLLOUT;
	if(oneshot)
		event.events |= EPOLLONESHOT;
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
		perror("epoll_ctl");
	setFdNonblocking(fd);
}

static void resetOneShot(int epollfd, int fd) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;
	if(epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) < 0)
		perror("epoll_ctl");
}

static void serverDelFd(Server* server, int fd) {
	if(epoll_ctl(server->priv, EPOLL_CTL_DEL, fd, NULL) < 0)
		perror("epoll_ctl");
}

static inline void handle(Server* server, int fd, struct sockaddr_in* addr) {
	int nread;
	char buff[20480];

	if((nread = recv(fd, buff, sizeof(buff), 0)) < 0) {
		if(errno == EAGAIN) {
			resetOneShot(server->priv, fd);
		}
		else {
			fprintf(stderr, "error: read failed\n");
		}
	}
	else if(nread > 0) {
		buff[nread] = '\0';

		Request* req = requestNew(buff);

		if(!req) {
			send(fd, "HTTP/1.0 400 Bad Request\r\n\r\nBad Request", 39, 0);
			pop_log(addr, "", "", 400);
		}
		else {
			Node* handler = server->handlers;
			Response* response = NULL;

			while(handler && !response) {
				response = (*(HandlerP)handler->value)(req);
				handler = handler->next;
			}

			if(!response) {
				send(fd, "HTTP/1.0 404 Not Found\r\n\r\nNot Found!", 36, 0);
				pop_log(addr, METHODS[req->method], req->path, 404);
			}
			else {
				pop_log(addr, METHODS[req->method], req->path, response->status);

				responseWrite(response, fd);
				responseDel(response);
			}

			requestDel(req);
		}
	}
	serverDelFd(server, fd);
	close(fd);
}

void serverServe(Server* server) {
	const int sock = makeSocket(server->port);
	struct epoll_event* events = malloc(sizeof(struct epoll_event) * EPOLL_NUM);

	serverAddFd(server->priv, sock, 1, false);

	fprintf(stdout, "Listening on port %d.\n\n", server->port);

	for(;;) {
		const int nfds = epoll_wait(server->priv, events, EPOLL_NUM, -1);
		for(int i = 0; i < nfds; ++i) {
			struct epoll_event event = events[i];
			const int tmpfd = event.data.fd;
			struct sockaddr_in addr;
			if(tmpfd == sock) {
				socklen_t size = sizeof(addr);
				int newSock = 0;
				while((newSock = accept(sock, (struct sockaddr*)&addr, &size)) > 0) {
					serverAddFd(server->priv, newSock, 1, true);
				}
				if(newSock == -1) {
					if(errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO
					   && errno != EINTR) {
						fprintf(stderr, "error: failed to accept connection\n");
						exit(1);
					}
				}
			}
			else {
				handle(server, tmpfd, &addr);
			}
		}
	}
}
