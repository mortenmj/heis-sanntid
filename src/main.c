#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <libheis/elev.h>

#include "comms.h"
#include "messages.h"
#include "queue.h"
#include "operator.h"
#include "orderLists.h"

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
	int target = 1;
	double floor = 1;
	int state;
	int ret;
	

	comms_create_socket();
    comms_set_nonblocking(fd);

	if (!elev_init()) {
		exit(1);
	}
	
	elev_reset_all_lamps();
	state = initializeOperator();
	
	elev_register_callback(SIGNAL_TYPE_CALL_UP, order_signal_callback);
	elev_register_callback(SIGNAL_TYPE_CALL_DOWN, order_signal_callback);
	elev_register_callback(SIGNAL_TYPE_COMMAND, order_signal_callback);

	ret = elev_enable_callbacks();
	
	while(1){
		printState( floor, state, target );
		floor=findFloor( floor, state );
		state=elevatorOperator(floor, &target, state);
        comms_listen();
        usleep(10000);
	};

	return 0;
}
