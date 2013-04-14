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
    COMMAND,
    CALLUP,
    CALLDOWN,
    N_TARGET
} targettype_t;

/* Return mode */
typedef enum {
    PRIORITY,
    EQUAL = 0,
    TARGET = 1,
    ORDERTYPE = 2,
    INPUT = 2,
    N_RETURNMODE_T = 2
} rmode_t;

/* Order types */
typedef enum {
    ORDER_UP,
    ORDER_DOWN,
    N_ORDER_TYPES
} order_type_t;

typedef struct {
	bool registered;
    bool updated;
} order_t;

typedef struct {
	//shared info between elevators
	priority_t priority;
	bool emergency_stop;
	double floor;
	bool synced;
    bool global_sync;
	// local info
	struct in_addr addr;
	struct in_addr elev_ready_for_sync;
	order_t commands[N_FLOORS]; 
} elevator_t;

typedef struct {
    unsigned long sender;
    order_t callUp[N_FLOORS-1];
    order_t callDown[N_FLOORS-1];
    elevator_t status;
} message_t;

typedef enum {
	UP,
	DOWN,
	WAIT,
	DOOR,
	STOP,
    N_STATES
} state_t;

#endif
