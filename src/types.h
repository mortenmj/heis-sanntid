#ifndef __TYPES_H__
#define __TYPES_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef enum {
    NONE,
    UPWARD,
    DOWNWARD,
    LOCKONOUTSIDE,
    N_PRIORITIES
} priority_t;

typedef enum {
    ORDER_UP,
    ORDER_DOWN,
    N_ORDER_TYPES
} order_type_t;

typedef enum {
    PRIORITY,
    EQUAL = 0,
    TARGET = 1,
    ORDERTYPE = 2,
    INPUT = 2,
    N_RMODES = 2
} rmode_t;
  
typedef struct {
    bool set;
    bool updated;
} order_t;

typedef struct {
    bool master;
    priority_t priority;
    bool emergency_stop;
    double floor;
    struct in_addr addr;
    order_t commands[N_FLOORS];
} elevator_t;

typedef struct {
    unsigned long sender;
    order_t call_up[N_FLOORS-1];
    order_t call_down[N_FLOORS-1];
    elevator_t elevator;
} message_t;

#endif
