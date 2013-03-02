#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libheis/elev.h>

#include "comms.h"
#include "messages.h"
#include "queue.h"
#include "operator.h"
#include "orderlist.h"
#include "target.h"

#define N_ORDERS 3

bool orders[N_ORDERS][N_FLOORS];
void command_signal_callback (int floor, int value);
int fdout, fdin;
int target;

int main (void)
{
    int my_id=0;
	double floor = -1;
	int ret;
	int numbytes;
	int target = -1;

	if (!elev_init()) {
        printf("elev_init failed\n");
		exit(1);
	}
	
	elev_reset_all_lamps();

	if (!orderlist_init()) {
        printf("orderlist_init failed\n");
        exit(1);
    }

	if (!operator_init()) {
        printf("operator_init failed\n");
        exit(1);
    }

    if (!elev_enable_callbacks()) {
        printf("elev_enable_callbacks failed\n");
        exit(1);
    }
	
	operator_start_elev();

	fdout = comms_create_out_socket();
	fdin = comms_create_in_socket();

    comms_set_nonblocking(fdin);

	while(1) {
		orderlist_register_local_orders();
        orderlist_sync(fdin);
        orderlist_clear_update_flag();

        floor = operator_get_floor();
        target = target_update(floor);
        
		operator_elev(floor, target);
		orderlist_set_lights();

		//operator_print_state(floor, target);
	}

	return 0;
}
