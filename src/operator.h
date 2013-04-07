typedef enum {
	UP,
	DOWN,
	WAIT,
	DOOR,
	STOP,
    N_STATES
} state_t;

state_t operator_elev (double floor, int *ptarget , state_t state);
state_t operator_init();
double operator_find_floor (double floorDouble, state_t state);
void operator_print_state (double floorDouble, state_t state, int target);
