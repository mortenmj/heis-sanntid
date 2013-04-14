#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#include "libheis/elev.h"
#include "operator.h"
#include "target.h"

//Private variables:

long int systemStartTime;

extern int my_id;

bool obstructionStored = 0;
bool stopSignal = 0;
double floorStored = -1;
state_t state;

pthread_mutex_t mutexFloorStored;
pthread_mutex_t mutexObstructionStored;
pthread_mutex_t mutexStopSignal;
pthread_mutex_t mutexState;

//Private functions:

static state_t
operator_get_state (void)
{
    pthread_mutex_lock(&mutexState);
    state_t s = state;
    pthread_mutex_unlock(&mutexState);

    return s;
}

static void
operator_set_state (state_t s) {
    pthread_mutex_lock(&mutexState);
    state = s;
    pthread_mutex_unlock(&mutexState);
}

static void
operator_set_stop_signal (int dummy1, int dummy2) {
	pthread_mutex_lock(&mutexStopSignal);
	stopSignal = 1;
	pthread_mutex_unlock(&mutexStopSignal);
}

static void
operator_store_obstruction_signal (int dummy, int value) {
	pthread_mutex_lock(&mutexObstructionStored);
	obstructionStored = value;
	pthread_mutex_unlock(&mutexObstructionStored);
}

static void
operator_store_floor_signal (int floor, int value) {
    state_t s = operator_get_state();

	pthread_mutex_lock(&mutexFloorStored); 

	if (value) {
		floorStored = floor;
		elev_set_floor_indicator(floor);
	} else {
		if (s == STATE_UP) {
			floorStored += 0.5;
		} else if (s == STATE_DOWN) {
			floorStored -= 0.5;
		}
	}

	pthread_mutex_unlock(&mutexFloorStored);
}

double
operator_get_floor (void) {
	pthread_mutex_lock(&mutexFloorStored);
	double temp = floorStored;
	pthread_mutex_unlock(&mutexFloorStored);

	return temp;
}

static bool
operator_get_stop_signal (void) {
	pthread_mutex_lock(&mutexStopSignal);
	bool temp = stopSignal;
	pthread_mutex_unlock(&mutexStopSignal);

	return temp;
}

static void
operator_reset_stop_signal (void) {
	pthread_mutex_lock(&mutexStopSignal);	
	stopSignal = 0;
	pthread_mutex_unlock(&mutexStopSignal);
}

static bool
operator_get_obstruction_signal (void) {
	pthread_mutex_lock(&mutexObstructionStored);	
	bool temp=obstructionStored;
	pthread_mutex_unlock(&mutexObstructionStored);

	return temp;
}

static void
operator_print_system_runtime (int mode) {
	int systemRunTime = time(NULL) - systemStartTime;
	int hours  = systemRunTime/3600;
	int min = (systemRunTime%3600)/60;
	int sec = (systemRunTime%3600)%60;

	if (mode == 0) {
		printf("SYSTEM RUN TIME: %i HOURS, %i MINUTES AND %i SECOUNDS.\n", hours,min,sec);
	} else {
		printf("\tRun time: \t%i:%i:%i\n", hours,min,sec);
	}
}

static void 
operator_stop_elev(void) {
	elev_set_speed(-SPEED);
	usleep(WAITTIME);
	elev_set_speed(SPEED);
	usleep(WAITTIME);
	elev_set_speed(0);
}

// Public functions:

int
operator_init (void) {
	elev_register_callback( SIGNAL_TYPE_SENSOR, &operator_store_floor_signal );
	elev_register_callback( SIGNAL_TYPE_STOP, &operator_set_stop_signal );
	elev_register_callback( SIGNAL_TYPE_OBSTR, &operator_store_obstruction_signal);

    operator_set_state(STATE_WAIT);
	systemStartTime = time(NULL);

    return state;
}

void
operator_start_elev (void) {
    if (operator_get_floor() == -1){
   	    elev_set_speed(SPEED);
   	    int stop = 0;
	    while (operator_get_floor() == -1){		//Runs until floor reach
		    if( operator_get_stop_signal() && stop == 0 ){ 		//emergency stop
		        stop = 1;
			    operator_stop_elev();
			    operator_reset_stop_signal();
			    printf("Press glowing button to start\n");
			    elev_set_stop_lamp(1); //turns on glowing button
		    }
		    if(operator_get_stop_signal() && stop){ 				//emergency start...
			    stop = 0;
			    operator_reset_stop_signal();
			    elev_set_stop_lamp(0);
			    elev_set_speed(SPEED);
		    }
	    }
	    operator_stop_elev();
    }
}

int
operator_elev (double floor, int target) {  // tar in ptarget i stedenfor target for og kunne teste operatoren
	static int doorTime; 
    state_t s = operator_get_state();

	if (operator_get_stop_signal() != 0 && state != STATE_STOP) {
		s = STATE_STOP;
		operator_stop_elev();
		elev_set_stop_lamp(1);

	} else {
		switch(state) {
			case STATE_UP:
				if (target == floor) {
					s = STATE_WAIT;
					operator_stop_elev();
				}else if ( target == -1) {
					if (fmod(floor,1) == 0) {
						s = STATE_WAIT;
						operator_stop_elev();
					}
				} else if ( target < floor){
					exit(1);
				}
			break;

			case STATE_DOWN:
				if (target == floor && target > -1) {
					s = STATE_WAIT;
					operator_stop_elev();
				} else if( target == -1) {
					if (fmod(floor,1) == 0) {
						s = STATE_WAIT;
						operator_stop_elev();
					}
				} else if ( target > floor ){
					exit(1);
				}
			break;

			case STATE_WAIT:
                //goes back into DOOR if obstruction.
				if (operator_get_obstruction_signal() == 0) {
					s = STATE_DOOR;
					doorTime = time(NULL);
					elev_set_door_open_lamp(1);
				} else {
					if (target == floor) {
						doorTime = time(NULL);
						s = STATE_DOOR;
						elev_set_door_open_lamp(1);
                    }

					if (target > floor) {
                        s = STATE_UP;
						elev_set_speed(SPEED);
					}

					if (target < floor && target > -1) {
						s = STATE_DOWN;
						elev_set_speed(-SPEED);
					}
				}
			break;

			case STATE_DOOR:
				if (!operator_get_obstruction_signal())
					doorTime = time(NULL);

				if ((int) (time(NULL)) >= doorTime + DOORTIME) { 	//waits DOORTIME secounds befor closing
					s = STATE_WAIT;
					elev_set_door_open_lamp(0);
					if (target == floor)					
						target_clear_completed_order();				// trenger en funktion som sier fra at at target har blit betjent
				}
			break;

			case STATE_STOP:
                //if elevator has a target, then it will go out of NØD_STOP
				if (target > floor && target != -1) {
					s = STATE_UP;
					operator_reset_stop_signal();
					elev_set_stop_lamp(0);
					elev_set_speed(SPEED);
					elev_set_door_open_lamp(0);
				} else if (target < floor && target != -1) {
					s = STATE_DOWN;
					operator_reset_stop_signal();
					elev_set_stop_lamp(0);
					elev_set_speed(-SPEED);
					elev_set_door_open_lamp(0);
				} else if (target == floor && target != -1) {
					s = STATE_WAIT;
					operator_reset_stop_signal();
					elev_set_stop_lamp(0);
				}
			break;

            default:
              s = STATE_WAIT;
		}
	}

    //printf("setting state to %d\n", s);
    operator_set_state(s);

    return 0;
}

/*
 * operator_set_stop_signal
 * floor: The current floor
 * state: The current state of the system
 * target: The target of the system
 *
 * Function to pretty print the state of the system
 *
 */
void 
operator_print_state (double floor, int target) {
    state_t s = operator_get_state();
    
    if (s == STATE_UP)
        printf("\ts: \t\tUP\n");
    else if (s == STATE_DOWN)
        printf("\ts: \t\tDOWN\n");
    else if (s == STATE_WAIT)
        printf("\ts: \t\tWAIT\n");
    else if (s == STATE_DOOR)
        printf("\ts: \t\tDOOR\n");
    else if (s == STATE_STOP)
        printf("\ts: \t\tSTOP\n");


    printf("\tTarget:\t\t%i \n", target);
    printf("\tCurrent floor:\t%f \n", floor);


    operator_print_system_runtime(1);

    printf("\tN_FLOORS: \t%i\n", N_FLOORS);
    printf("\tDoor time: \t%i sec\n", DOORTIME);
    if (s == STATE_UP)
        printf("\tSpeed:\t\t%i\n", SPEED);
    else if (s == STATE_DOWN)
        printf("\tSpeed:\t\t%i\n", -SPEED);
    else
        printf("\tSpeed:\t\t0\n");
    printf("------------------------------------\n");
}
