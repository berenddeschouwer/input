#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "input.h"
#include "tcp.h"

/* ------------------------------------------------------------------ */

static int usage(char *prog, int error)
{
	fprintf(error ? stderr : stdout,
		"usage: %s"
		"\n",
		prog);
	exit(error);
}

int main(int argc, char *argv[])
{
	char *addr   = NULL;
	char *port   = NULL;
	char *host   = "localhost";
	char *serv   = "1234";
	struct addrinfo ask;
	int c,sock;

	memset(&ask,0,sizeof(ask));
	ask.ai_family = PF_UNSPEC;
	ask.ai_socktype = SOCK_STREAM;

	for (;;) {
		if (-1 == (c = getopt(argc, argv, "h")))
			break;
		switch (c) {
		case 'h':
			usage(argv[0],0);
		default:
			usage(argv[0],1);
		}
	}

	tcp_verbose = 1;
	sock = tcp_connect(&ask,addr,port,host,serv);
	if (-1 == sock)
		exit(1);

	for (;;) {
		struct input_event ev;
		fd_set set;
		int rc;

		FD_ZERO(&set);
		FD_SET(sock,&set);
		rc = select(sock+1,&set,NULL,NULL,NULL);
		if (1 != rc) {
			perror("select");
			exit(1);
		}
		
		rc = read(sock,&ev,sizeof(ev));
		if (rc != sizeof(ev)) {
			fprintf(stderr,"read ret=%d (expected %d), errno=%s\n",
				rc,(int)sizeof(ev),strerror(errno));
			exit(1);
		}
		
		/* convert from network byte order ... */
		ev.time.tv_sec  = ntohl(ev.time.tv_sec);
		ev.time.tv_usec = ntohl(ev.time.tv_usec);
		ev.type         = ntohs(ev.type);
		ev.code         = ntohs(ev.code);
		ev.value        = ntohl(ev.value);

		print_event(&ev);
	}
		
	return 0;
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
