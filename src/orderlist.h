#ifndef __ORDERLIST_H__
#define __ORDERLIST_H__

#define N_FLOORS 4
#define N_LAMPS 3
#define MAX_N_ELEVATORS 3

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

typedef struct {
	bool registered;
    bool updated;
} orderinfo_t;

typedef struct {
	//shared info between elevators
	priority_t priority;
	bool emergency_stop;
	double floor;
	// local info
	struct in_addr addr;
	struct in_addr elev_ready_for_sync;
	bool synced;
    bool global_sync;
	orderinfo_t commands[N_FLOORS]; 
} elevstatus_t;

int orderlist_init (void);
void orderlist_register_local_orders (void);
void orderlist_clear_update_flag (void);
void orderlist_set_lights (void);
elevstatus_t* orderlist_get_local_status (void);
int orderlist_sync (int fd);
void orderlist_print_lists(void);

#endif
