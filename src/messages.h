#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <time.h>
#include <stdbool.h>

#include "orderlist.h"

extern orderinfo_t callUp[ N_FLOORS-1 ]; 					// 0 = floor 1, N_FLOORS = floor N_FLOORS - 1.
extern orderinfo_t callDown[ N_FLOORS-1 ];					// 0 = floor 2, N_FLOORS = floor N_FLOORS.
extern orderinfo_t commands[ MAX_N_ELEVATORS ][ N_FLOORS ];
extern int targets[MAX_N_ELEVATORS];

typedef struct {
    int type;
	int floor;
    time_t timestamp;
} message_t;

typedef enum {
    HEARTBEAT,
    ORDER_COMPLETE,
    ORDER_REGISTERED,
    N_ORDER_TYPES
} order_type_t;


char* message_create (message_t msg);
message_t message_parse (char *msg);

void message_print (message_t msg);

#endif
