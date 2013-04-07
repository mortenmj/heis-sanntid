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

typedef struct {
	int time_registered;             //dette må sendes
	int targeted;                   //dette må sendes
	bool registered;                //dette må sendes
} orderinfo_t;

typedef struct {
	priority_t priority;            //dette må sendes
	bool emergency_stop;             //dette må sendes
	double floor;                   //dette må sendes
	int ip;
	int time_registered;            //dette må sendes?
	int target;                     //dette må sendes
	int locked_target;              //dette må sendes
	orderinfo_t commands[N_FLOORS]; //man trenger kunn og sende egene commands
} elevstatus_t;

void orderlist_init (void);
void orderlist_register_local_orders();
void orderlist_clear_targeteted_order (int target);
int orderlist_simple_update_targets();
void orderlist_set_lights ();

#endif
