#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <time.h>
#include <stdbool.h>

#include "orderlist.h"

extern order_t callUp[N_FLOORS-1];
extern order_t callDown[N_FLOORS-1];
extern elevator_t elevators[MAX_N_ELEVATORS];

char* message_create_status (void);
int message_parse_status (message_t* m, char* msg);

#endif
