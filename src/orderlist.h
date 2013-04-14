#ifndef __ORDERLIST_H__
#define __ORDERLIST_H__

#define N_FLOORS 4
#define N_LAMPS 3
#define MAX_N_ELEVATORS 3

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "types.h"

int orderlist_init (void);
void orderlist_register_local_orders (void);
void orderlist_clear_update_flag (void);
void orderlist_set_lights (void);
elevator_t *orderlist_get_local_elevator (void);
order_t orderlist_get_local_order (int floor, order_type_t type);
void orderlist_set_local_order (int floor, order_type_t type, bool value);
void orderlist_sync (int fd);
void orderlist_print_lists(void);
void orderlist_commands_to_file ();

#endif
