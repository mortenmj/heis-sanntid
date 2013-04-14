#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
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
#include "timeout.h"

#define N_ORDERS 3
#define N_FLOORS 4

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

	orderlist_init();

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

    struct timeval diff, time1, time2;
    gettimeofday(&time1, NULL);

    /* Look for master node */
    while (diff.tv_sec < 2) {
        char* msg;
        int numbytes;
        message_t *m = malloc(sizeof(message_t));

        gettimeofday (&time2, NULL);
        timeval_subtract(&diff, &time2, &time1);

        /* Send data */
        msg = message_create_status();
        //comms_send_data(msg);

        /* Receive data */
        numbytes = comms_listen (fdin, &msg);
        if (numbytes > 0) {
            msg[numbytes] = '\0';
            message_parse_status(m, msg);

            /* Another elevator is already set as master */
            if (m->elevator.master) {
                printf("Existing master found\n");
                break;
            }
        }

        /* We've timed out without finding a master, so we become master */
        if (diff.tv_sec == 2) {
            orderlist_set_master_status(true);
            printf("No master found, becoming master: %d\n", orderlist_get_master_status());
        }
  
        usleep(1000);
        free(m);
    }

	while(1) {
        char* msg;
        int numbytes;
        message_t *m = malloc(sizeof(message_t));

        numbytes = comms_listen (fdin, &msg);

        /* Listen to incoming messages from master */
        if (numbytes > 0) {
            msg[numbytes] = '\0';
            message_parse_status(m, msg);

            if (m->elevator.master) {
                orderlist_update(m);
            }

            free (m);
        }
        /*
		orderlist_register_local_orders();
        orderlist_sync(fdin);
        orderlist_clear_update_flag();

        floor = operator_get_floor();
        target = target_update(floor);
        
		operator_elev(floor, target);
		orderlist_set_lights();

		//operator_print_state(floor, target);
        */
	}

	return 0;
}
