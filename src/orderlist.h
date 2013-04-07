#ifndef __ORDERLIST_H__
#define __ORDERLIST_H__

#define N_FLOORS 4
#define MAX_N_ELEVATORS 3

typedef enum {
	NONE,
	UPWARD,
	DOWNWARD,
	LOCKONOUTSIDE,
    N_PRIORITIES
} priority_t;

typedef enum {
	UP_DIR,
	DOWN_DIR,
    N_DIRS
} dir_t;

typedef struct {
	int timeRegistered;
	bool targeted;
	bool registered;
} orderinfo_t;

typedef struct {
	priority_t priority;
	bool registered;
	bool emergencyStop;
    double floor;
} elevstatus_t;

void orderlist_init (void);
int orderlist_register_elev (void);
void orderlist_delete_elev (int elevator);
void orderlist_clear_targeted_order (int target , int elevator, dir_t elevatorDir);
int orderlist_register_call_up (int button);
int orderlist_register_call_down (int button);
int orderlist_register_call_order (int floor, int elevator);
void orderlist_set_lights (double floor, int elevator);

#endif
