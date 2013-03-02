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

int operator_elev (double floor, int target);
int operator_init (void);
void operator_start_elev (void);
//double operator_find_floor (double floorDouble, state_t state);
void operator_print_state (double floorDouble, int target);
double operator_get_floor (void);
