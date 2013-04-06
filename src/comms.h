#include <netinet/in.h>

int comms_create_socket (void);
int comms_listen (void);
int comms_send_data (unsigned char *msg);
void comms_set_blocking (int fd);
void comms_set_nonblocking (int fd);
