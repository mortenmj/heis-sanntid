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
#include "orderlist.h"

#define N_ORDERS 3

bool orders[N_ORDERS][N_FLOORS];
void command_signal_callback (int floor, int value);
int fd;

static void order_signal_callback (int floor, int value) {
    unsigned char* msg = messages_create_order(floor, value);

    comms_send_data(msg);
}

int main()
{
	int target = 1;
	double floor = 1;
	int state;
	int ret;
    int numbytes;
    char* msg;
    order_t order;
	

	fd = comms_create_socket();
    comms_set_nonblocking(fd);

	if (!elev_init()) {
		exit(1);
	}
	
	elev_reset_all_lamps();
	state = operator_init();
	
	elev_register_callback(SIGNAL_TYPE_CALL_UP, order_signal_callback);
	elev_register_callback(SIGNAL_TYPE_CALL_DOWN, order_signal_callback);
	elev_register_callback(SIGNAL_TYPE_COMMAND, order_signal_callback);

	ret = elev_enable_callbacks();
	
	while(1){
        if ((numbytes = comms_listen(&msg)) > 0) {
            msg[numbytes] = '\0';
            order = messages_parse_order(msg);

            printf("order type: %d, order floor %d\n", order.type, order.floor);
        }

		floor = operator_find_floor(floor, state);
		state = operator_elev(floor,&target, state);

        //printf("%c[2J",27);
		//operator_print_state(floor,state, target);

        usleep(1000);
	};

	return 0;
}
