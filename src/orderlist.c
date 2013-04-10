#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <libheis/elev.h>

#include "orderlist.h"

int my_id;

orderinfo_t callUp[N_FLOORS-1];					    // 0 = floor 1, N_FLOORS = floor N_FLOORS - 1.
orderinfo_t callDown[N_FLOORS-1];					    // 0 = floor 2, N_FLOORS = floor N_FLOORS.
elevstatus_t elevators[MAX_N_ELEVATORS];

static bool callUp_mutexed[N_FLOORS-1];				// 0 = floor 1, N_FLOORS = floor N_FLOORS - 1.
static bool callDown_mutexed[N_FLOORS-1];				// 0 = floor 2, N_FLOORS = floor N_FLOORS.
static bool commands_mutexed[N_FLOORS];

pthread_mutex_t mutex_callUp;
pthread_mutex_t	mutex_callDown;
pthread_mutex_t mutex_commands;

//=====================================================PRIVATE FUNCTIONS==================================================

static void 
orderlist_mutexed_get_call_up (int button, int dummy) {
	pthread_mutex_lock(&mutex_callUp);
	callUp_mutexed[button] = true;
	pthread_mutex_unlock(&mutex_callUp);
}

static void
orderlist_mutexed_get_call_down (int floor, int dummy) {
	pthread_mutex_lock(&mutex_callDown);
	callDown_mutexed[floor-1] = true;
	pthread_mutex_unlock(&mutex_callDown);
}

static void
orderlist_mutexed_get_command (int button, int dummy) {
	pthread_mutex_lock(&mutex_commands);
	commands_mutexed[button] = true;
	pthread_mutex_unlock(&mutex_commands);
}

//=========================================================== PUBLIC FUNCTIONS =================================================
// dir bestemmer om callUp eller CalDown skal slettes,

void
orderlist_clear_targeteted_order (int target) {
	if ( target < (N_FLOORS - 1) && elevators[my_id].priority == UPWARD ) {
		callUp[target].time_registered = -1;
		callUp[target].targeted = -1;
		callUp[target].registered = false;


	} else if (target > 0 && elevators[my_id].priority == DOWNWARD) {
		callDown[target-1].time_registered = -1;
		callDown[target-1].targeted = -1;
		callDown[target-1].registered = false;
    }
    
	elevators[my_id].commands[target].time_registered = -1;
	elevators[my_id].commands[target].targeted = -1;
	elevators[my_id].commands[target].registered = false;
	elevators[my_id].target = -1;
}

void
orderlist_register_local_orders (void) {
	pthread_mutex_lock(&mutex_commands);
	
	for (int button=0; button < N_FLOORS; button++) {
		if (commands_mutexed[button]) {

			if (!elevators[my_id].commands[button].registered) {
				elevators[my_id].commands[button].time_registered = time(NULL);
				elevators[my_id].commands[button].targeted = -1;
			}

			elevators[my_id].commands[button].registered = true;
			commands_mutexed[button] = false;
		}
	}
	
	pthread_mutex_unlock(&mutex_commands);

	pthread_mutex_lock(&mutex_callDown);
	for (int button=0; button < N_FLOORS-1; button++){
		if (callDown_mutexed[button]) {
			if (!callDown[button].registered) {
				callDown[button].time_registered = time(NULL);
				callDown[button].targeted = -1;
			}

			callDown[button].registered = true;
			callDown_mutexed[button] = false;
		}
	}
	pthread_mutex_unlock(&mutex_callDown);


	pthread_mutex_lock(&mutex_callUp);
	for (int button=0; button < N_FLOORS-1; button++) {
		if (callUp_mutexed[button]) {
			if (callUp[button].registered == false) {
				callUp[button].time_registered = time(NULL);
				callUp[button].targeted = -1;
			}

			callUp[button].registered = true;
			callUp_mutexed[button] = false;
		}
	}
	pthread_mutex_unlock(&mutex_callUp);
}

void
orderlist_set_lights (void) {
	for (int i = 0; i < (N_FLOORS -1); i++){
        elev_set_button_lamp(ELEV_DIR_UP, i, callUp[i].registered );
        elev_set_button_lamp(ELEV_DIR_DOWN, i + 1, callDown[i].registered );
        elev_set_button_lamp(ELEV_DIR_COMMAND, i, elevators[my_id].commands[i].registered );
	}

	elev_set_button_lamp(ELEV_DIR_COMMAND, (N_FLOORS - 1), elevators[my_id].commands[N_FLOORS - 1].registered );
}

void
orderlist_init (void) {
	for (int i=0; i < (N_FLOORS-1) ; i++){
		callUp[i].time_registered = 0;
		callUp[i].registered = false;
		callUp[i].targeted = -1;
		callUp_mutexed[i] = false;

		callDown[i].time_registered = 0;
		callDown[i].registered = false;
		callDown[i].targeted = -1;
		callDown_mutexed[i] = false;

		commands_mutexed[i] = false;
	}
	commands_mutexed[N_FLOORS-1] = false;

	for (int elev=0; elev < MAX_N_ELEVATORS; elev++){
		for (int i=0; i < N_FLOORS; i++){
			elevators[elev].commands[i].time_registered = 0;
			elevators[elev].commands[i].registered = false;
			elevators[elev].commands[i].targeted = -1;
		}

		elevators[elev].emergency_stop = false;
		elevators[elev].priority = NONE;
		elevators[elev].floor = 0;
		elevators[elev].target = -1;
		elevators[elev].locked_target = -1;
		elevators[elev].time_registered = -1;
		elevators[elev].ip = -1;

	}

	elev_register_callback( SIGNAL_TYPE_CALL_UP, &orderlist_mutexed_get_call_up );
	elev_register_callback( SIGNAL_TYPE_CALL_DOWN, &orderlist_mutexed_get_call_down );
	elev_register_callback( SIGNAL_TYPE_COMMAND, &orderlist_mutexed_get_command );
}

int
orderlist_simple_update_targets (void) {
    for (int elevator = 0; elevator< MAX_N_ELEVATORS; elevator++) {

        if (elevators[elevator].time_registered == -1 ) {
            break;
        } else if(elevators[elevator].target == -1) {
            for(int floor = 0; floor < N_FLOORS; floor++) {
                if( elevators[elevator].commands[floor].registered && elevators[elevator].commands[floor].targeted == -1 ) {
                    
                    elevators[elevator].commands[floor].targeted = elevator;
                    elevators[elevator].priority = NONE;
                    elevators[elevator].target = floor;
                    printf("command\n");

                    return elevators[my_id].target;
                }
            }
            for (int floor = 0; floor < N_FLOORS-1; floor++) {
                if (callUp[floor].registered && callUp[floor].targeted == -1) {
                    callUp[floor].targeted = elevator;
                    elevators[elevator].target = floor;
                    elevators[elevator].priority = UPWARD;
                    printf("callUp\n");

                    return elevators[my_id].target;
                }
            }
            for (int floor = 0; floor < N_FLOORS-1; floor++) {
                if (callDown[floor].registered && callDown[floor].targeted == -1) {
                    callDown[floor].targeted = elevator;
                    elevators[elevator].target = floor+1;
                    elevators[elevator].priority = DOWNWARD;
                    printf("callDown\n");

                    return elevators[my_id].target;
                }
            }
        }
    }
    
    return elevators[my_id].target;
}
