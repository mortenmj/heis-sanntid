#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#define BROADCAST_PORT 23981
#define BROADCAST_PORT_STRING "23981"
#define BROADCAST_GROUP "129.241.187.255"
#define BUFLEN 4096

int fdout, fdin;
struct sockaddr_in addr;

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* comms_set_blocking:
 * socket: Socket descriptor
 *
 * Sets socket as blocking.
 *
 */
void
comms_set_blocking (int sock) {
  int flags;

  flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
}

/*
 * comms_set_nonblocking:
 * socket: Socket descriptor
 *
 * Sets socket as non-blocking.
 *
 */
void
comms_set_nonblocking (int sock) {
  int flags;

  flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

int
comms_create_out_socket (void) {
    socklen_t addrlen;

    if ((fdout = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("sock");
        exit(1);
    }

    int broadcast = 1;
    if (setsockopt(fdout, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    addrlen = sizeof addr;

    memset(&addr, 0, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(BROADCAST_GROUP);
    addr.sin_port = htons(BROADCAST_PORT);

    return fdout;
}

int
comms_create_in_socket (void) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	socklen_t addrlen;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, BROADCAST_PORT_STRING, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	} 
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

        addrlen = sizeof addr;
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

    return sockfd;
}

/*
 * comms_listen:
 *
 * Checks socket for new data
 *
 * Returns: Number of bytes read
 *
 */
int
comms_listen (int fd, char** msg) {
    int numbytes;
	char buf[BUFLEN];
	struct sockaddr_storage their_addr;
    socklen_t addrlen;
  
    addrlen = sizeof addr;
    numbytes = recvfrom(fd, buf, BUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addrlen);

    if (numbytes == -1 && EAGAIN != errno && EWOULDBLOCK != errno) {
        perror("recvfrom");
        exit(1);
    }

    *msg = buf;

	return numbytes;
}

/*
 * comms_send_data:
 * socket: Socket descriptor
 * data: The data to be sent
 * len: Length of the data to be sent *
 * Function to send data over a socket.
 *
 * Returns: Number of bytes sent
 */

int
comms_send_data (unsigned char* msg) {
    int bytes_sent = 0;
    int len = strlen(msg);
    socklen_t addrlen;

    if (sizeof(msg) == 0) {
        return 0;
    }

    addrlen = sizeof addr;
    bytes_sent = sendto(fdout, msg, len, 0, (struct sockaddr *) &addr, sizeof(addr));

    return bytes_sent;
}
