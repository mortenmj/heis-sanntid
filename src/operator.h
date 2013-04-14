#define SPEED 1000
#define WAITTIME 10000
#define DOORTIME 3

int operator_init (void);
void operator_start (void);
void operator_update (double floor, int target);
void operator_print_state (double floorDouble, int target);
double operator_get_floor (void);
