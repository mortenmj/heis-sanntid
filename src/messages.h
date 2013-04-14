#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#define N_FLOORS 4

#include <time.h>
#include <stdbool.h>

#include "types.h"
#include "orderlist.h"

char* message_create_status (void);
int message_parse_status (message_t* m, char* msg);

#endif
