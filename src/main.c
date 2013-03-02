#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libheis/elev.h>

#include "comms.h"

#define N_ORDERS 3

bool orders[N_ORDERS][N_FLOORS];
void command_button_callback (int floor, int value);

int main()
{
	int ret;

    comms_create_socket();

	if (!elev_init()) {
		exit(1);
	}

	elev_register_callback(SIGNAL_TYPE_COMMAND, command_button_callback);
	ret = elev_enable_callbacks();
	
	while(1);

	return 0;
}

void command_button_callback (int floor, int value) {
    unsigned char* msg = "lolollhaha";

    comms_send_data(msg);
}
