#include <netinet/in.h>

void comms_create_socket (void);
int comms_send_data (unsigned char *msg);
int comms_set_blocking (void);
int comms_set_nonblocking (void);
