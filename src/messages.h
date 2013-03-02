#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <time.h>
#include <stdbool.h>

#include "orderlist.h"

extern orderinfo_t callUp[N_FLOORS-1];
extern orderinfo_t callDown[N_FLOORS-1];
extern elevstatus_t elevators[MAX_N_ELEVATORS];

typedef struct {
    unsigned long sender;
    orderinfo_t callUp[N_FLOORS-1];
    orderinfo_t callDown[N_FLOORS-1];
    elevstatus_t status;
} message_t;

char* message_create_status (void);
int message_parse_status (message_t* m, char* msg);
//void message_print (message_t msg);

#endif
