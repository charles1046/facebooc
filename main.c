#include <signal.h>
#include <stdio.h>

#include "facebooc_server.h"

static void sig(int signum);
Facebooc* fb;

int main(int argc, char* argv[]) {
	if(signal(SIGINT, sig) == SIG_ERR || signal(SIGTERM, sig) == SIG_ERR) {
		fprintf(stderr, "error: failed to bind signal handler\n");
		return 1;
	}

	uint16_t server_port = 8080;

	if(argc > 1 && sscanf(argv[1], "%hu", &server_port) == 0) {
		fprintf(stderr, "error: invalid command line argument, using default port "
						"8080.\n");
		server_port = 8080;
	}

	fb = FB_new(server_port);
	int res = FB_run(fb);
	if(res) {
		// Do some error handling
		fprintf(stderr, "Stop fb server.\n");
	}
	FB_delete(fb);

	return 0;
}

static void sig(int signum) {
	FB_delete(fb);

	fprintf(stdout, "\n[%d] Bye!\n", signum);
	exit(0);
}