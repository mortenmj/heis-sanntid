#ifndef __COMMS_H__
#define __COMMS_H__

#include <netinet/in.h>

int comms_create_out_socket (void);
int comms_create_in_socket (void);
int comms_listen (int fd, char** msg);
int comms_send_data (unsigned char *msg);
void comms_set_blocking (int fd);
void comms_set_nonblocking (int fd);
struct in_addr comms_get_address (void);

#endif
