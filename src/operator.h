#define SPEED 1000
#define WAITTIME 10000
#define DOORTIME 3

typedef enum {
	UP,
	DOWN,
	WAIT,
	DOOR,
	STOP,
    N_STATES
} state_t;

state_t operator_elev (double floor, int target , state_t state);
state_t operator_init();
void operator_start_elev();
double operator_find_floor (double floorDouble, state_t state);
void operator_print_state (double floorDouble, state_t state, int target);
