//BUILD AN ENTIRE SOCKET LIBRARY (MODULE) WITH METHODS LIKE 
// TODO  CREATE_SOCKET(...), 
//  RECEIVE(...), 
//  SEND(...), 
//  CONNECT(...), 
//  LISTEN(...), 
//  SET_NONBLOCKING()
//  SET_BLOCKING(...)
//

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BROADCAST_PORT 23981
#define BROADCAST_GROUP "129.241.187.255"

int sockfd;
struct sockaddr_in addr;

void
comms_create_socket (void)
{
  int broadcast = 1;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      perror("socket");
      exit(1);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {
      perror("setsockopt (SO_BROADCAST)");
      exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(BROADCAST_GROUP);
  addr.sin_port = htons(BROADCAST_PORT);
}

/*
 * comms_send_data:
 * socket: Socket descriptor
 * data: The data to be sent
 * len: Length of the data to be sent
 *
 * Function to send data over a socket.
 *
 * Returns: Number of bytes sent
 */
int
comms_send_data (unsigned char* msg)
{
  int bytes_sent = 0;
  int len = strlen(msg);

  if (sizeof(msg) == 0) {
      return 0;
  }

  bytes_sent = sendto(sockfd, msg, len, 0, (struct sockaddr *) &addr, sizeof(addr));

  return bytes_sent;
}

/* comms_set_blocking:
 * socket: Socket descriptor
 *
 * Sets socket as blocking.
 *
 * Returns: 1 for success or 0.
 */
int
comms_set_blocking (void)
{
  int opts, ret;

  opts = fcntl(sockfd, F_GETFL, 0);

  if (opts < 0) {
      opts = 0;
  }
  
  opts |= O_NONBLOCK;
  ret = fcntl(sockfd, F_SETFL, opts);
  
  if (ret == -1)
    return 0;
  return 1;
}

/*
 * comms_set_nonblocking:
 * socket: Socket descriptor
 *
 * Sets socket as non-blocking.
 *
 * Returns: 1 for success or 0.
 */
int
comms_set_nonblocking (void)
{
  int opts, ret;

  opts = fcntl(sockfd, F_GETFL, 0);

  if (opts < 0) {
      opts = 0;
  }
  
  opts &= ~O_NONBLOCK;
  ret = fcntl(sockfd, F_SETFL, opts);
  
  if (ret == -1)
    return 0;
  return 1;
}
