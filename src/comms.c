//BUILD AN ENTIRE SOCKET LIBRARY (MODULE) WITH METHODS LIKE 
// TODO  CREATE_SOCKET(...), 
//  RECEIVE(...), 
//  SEND(...), 
//  CONNECT(...), 
//  LISTEN(...), 
//  SET_NONBLOCKING()
//  SET_BLOCKING(...)
//

#include <unistd.h>
#include <fcntl.h>

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
comms_send_data (int socket,
    unsigned char* data,
    unsigned long len)
{
  int ret, sent, tmp;

  ret = 0;
  sent = 0;

  if (len != 0) {
      while (sent < len) {
          /* send() returns number of bytes sent or -1 on error */
          tmp = send(socket, &data[sent], len-sent, NULL);
          
          if (tmp < 0) {
              ret = -1;
              break;
          } else {
              sent += tmp;
              ret = sent;
          }
      }
  }

  return ret;
}

/* comms_set_blocking:
 * socket: Socket descriptor
 *
 * Sets socket as blocking.
 *
 * Returns: 1 for success or 0.
 */
int
comms_set_blocking (int socket)
{
  int opts, ret;

  opts = fcntl(socket, F_GETFL, 0);

  if (opts < 0) {
      opts = 0;
  }
  
  opts |= O_NONBLOCK;
  ret = fcntl(socket, F_SETFL, opts);
  
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
comms_set_nonblocking (int socket)
{
  int opts, ret;

  opts = fcntl(socket, F_GETFL, 0);

  if (opts < 0) {
      opts = 0;
  }
  
  opts &= ~O_NONBLOCK;
  ret = fcntl(socket, F_SETFL, opts);
  
  if (ret == -1)
    return 0;
  return 1;
}
