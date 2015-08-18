#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "list.h"
#include "input.h"
#include "tcp.h"

struct connection {
	int                      socket;
	struct sockaddr_storage  peer;
	struct list_head         list;
	char                     peerhost[INET6_ADDRSTRLEN+1];
	char                     peerserv[33];
};

/* ------------------------------------------------------------------ */

int debug   = 0;
int timeout = 10;
int slisten;
int input;
LIST_HEAD(connections);

static void conn_new(void)
{
	struct connection  *conn;
	unsigned int len;

	conn = malloc(sizeof(conn));
	memset(conn,0,sizeof(conn));
	len = sizeof(conn->peer);
	conn->socket = accept(slisten,(struct sockaddr*)&conn->peer,&len);
	if (-1 == conn->socket) {
		if (tcp_verbose)
			perror("accept");
		free(conn);
		return;
	}
	if (tcp_verbose) {
		getnameinfo((struct sockaddr*)&conn->peer,len,
			    conn->peerhost,INET6_ADDRSTRLEN,
			    conn->peerserv,32,
			    NI_NUMERICHOST | NI_NUMERICSERV);
		fprintf(stderr,"connect from [%s]\n",conn->peerhost);
	}
	fcntl(conn->socket,F_SETFL,O_NONBLOCK);

	/* FIXME: access control */
	list_add_tail(&conn->list,&connections);
}

static void conn_del(struct connection  *conn)
{
	if (tcp_verbose)
		fprintf(stderr,"connection from [%s] closed\n",conn->peerhost);
	close(conn->socket);
	list_del(&conn->list);
	free(conn);
}

static void input_bcast(struct input_event *ev)
{
	struct connection  *conn;
	struct list_head   *item;
	struct list_head   *safe;
	int rc;

	if (debug)
		print_event(ev);
	
	/* convert to network byte order ... */
	ev->time.tv_sec  = htonl(ev->time.tv_sec);
	ev->time.tv_usec = htonl(ev->time.tv_usec);
	ev->type         = htons(ev->type);
	ev->code         = htons(ev->code);
	ev->value        = htonl(ev->value);

	/* send out */
	list_for_each_safe(item,safe,&connections) {
		conn = list_entry(item, struct connection, list);
		rc = write(conn->socket,ev,sizeof(*ev));
		if (rc != sizeof(*ev))
			conn_del(conn);
	}
}

static void loop(void)
{
	struct connection  *conn;
	struct list_head   *item;
	struct list_head   *safe;
	fd_set             set;
	struct timeval     tv;
	int                max,rc;

	for (;;) {
		tv.tv_sec  = timeout;
		tv.tv_usec = 0;

		FD_ZERO(&set);
		FD_SET(slisten,&set);
		max = slisten;

		FD_SET(input,&set);
		if (max < input)
			max = input;

		list_for_each(item,&connections) {
			conn = list_entry(item, struct connection, list);
			FD_SET(conn->socket,&set);
			if (max < conn->socket)
				max = conn->socket;
		}

		rc = select(max+1,&set,NULL,NULL,&tv);
		if (rc < 0) {
			/* Huh? */
			perror("select");
			exit(1);
		}
		if (0 == rc) {
			/* timeout */
			continue;
		}

		list_for_each_safe(item,safe,&connections) {
			conn = list_entry(item, struct connection, list);
			if (FD_ISSET(conn->socket,&set)) {
				char dummy[16];
				rc = read(conn->socket,dummy,sizeof(dummy));
				if (rc <= 0)
					conn_del(conn);
			}
		}

		if (FD_ISSET(input,&set)) {
			struct input_event ev;
			rc = read(input,&ev,sizeof(ev));
			if (rc != sizeof(ev))
				exit(1);
			input_bcast(&ev);
		}

		if (FD_ISSET(slisten,&set))
			conn_new();
	}
}

/* ------------------------------------------------------------------ */

static void daemonize(void)
{
        switch (fork()) {
        case -1:
		perror("fork");
		exit(1);
        case 0:
		close(0); close(1); close(2);
		setsid();
		open("/dev/null",O_RDWR); dup(0); dup(0);
		break;
        default:
		exit(0);
        }
}

static int usage(char *prog, int error)
{
	fprintf(error ? stderr : stdout,
		"usage: %s"
		" [ -t <sec> ] [ -g ]"
		" devnr\n",
		prog);
	exit(error);
}

int main(int argc, char *argv[])
{
	int grab     =  0;
	char *addr   = NULL;
	char *port   = "1234";
	int c,devnr;
	struct addrinfo ask;

	memset(&ask,0,sizeof(ask));
	ask.ai_family = PF_UNSPEC;
	ask.ai_socktype = SOCK_STREAM;

	for (;;) {
		if (-1 == (c = getopt(argc, argv, "hdgt:")))
			break;
		switch (c) {
		case 'd':
			debug = 1;
			tcp_verbose = 1;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case 'g':
			grab = 1;
			break;
		case 'h':
			usage(argv[0],0);
		default:
			usage(argv[0],1);
		}
	}

	if (optind == argc)
		usage(argv[0],1);
	devnr = atoi(argv[optind]);
	input = device_open(devnr,debug);
	if (-1 == input)
		exit(1);

	slisten = tcp_listen(&ask,addr,port);
	if (-1 == slisten)
		exit(1);

	if (!debug)
		daemonize();
	loop();
	return 0;
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
