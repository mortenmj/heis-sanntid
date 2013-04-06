#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <string.h>
#include <time.h>

#include <libheis/elev.h>

#include "comms.h"
#include "messages.h"
#include "queue.h"

#define N_ORDERS 3

bool orders[N_ORDERS][N_FLOORS];
void command_signal_callback (int floor, int value);
int fd;
fd_set socks;

static void order_signal_callback (int floor, int value) {
    unsigned char* msg = messages_order(floor, value);

    comms_send_data(msg);
}

int main()
{
	int ret;
    pthread_t listener;
    fd_set tmpset;

    fd = comms_create_socket();
    comms_set_nonblocking(fd);

	if (!elev_init()) {
		exit(1);
	}

    pthread_create (&listener, NULL, (void *) &comms_listen, NULL);

	elev_register_callback(SIGNAL_TYPE_CALL_UP, order_signal_callback);
	elev_register_callback(SIGNAL_TYPE_CALL_DOWN, order_signal_callback);
	elev_register_callback(SIGNAL_TYPE_COMMAND, order_signal_callback);

	ret = elev_enable_callbacks();

	while(1) {
        comms_listen();
    }

	return 0;
}
