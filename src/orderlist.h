#ifndef __ORDERLIST_H__
#define __ORDERLIST_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "types.h"
#include "messages.h"

void orderlist_init (void);
void orderlist_update (message_t* m);
order_t orderlist_get_local_order (int floor, order_type_t type);
void orderlist_set_local_order (int floor, order_type_t type, bool value);
elevator_t orderlist_get_local_elevator (void);
void orderlist_set_master_status (bool status);
bool orderlist_get_master_status (void);

#endif
