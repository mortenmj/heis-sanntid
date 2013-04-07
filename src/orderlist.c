#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <libheis/elev.h>

#include "orderlist.h"


orderinfo_t callUp[ N_FLOORS-1 ]; 					// 0 = floor 1, N_FLOORS = floor N_FLOORS - 1.
orderinfo_t callDown[ N_FLOORS-1 ];					// 0 = floor 2, N_FLOORS = floor N_FLOORS.
orderinfo_t commands[ MAX_N_ELEVATORS ][ N_FLOORS ];

int targets[MAX_N_ELEVATORS];

elevstatus_t activeElevators[MAX_N_ELEVATORS];

void
orderlist_init (void) {
	for (int i; i < (N_FLOORS-1) ; i++){
		callUp[i].timeRegistered = 0;
		callUp[i].registered = false;
		callUp[i].targeted = false; 					

		callDown[i].timeRegistered = 0;
		callDown[i].registered = false;
		callDown[i].targeted = false;
	}

	for (int j; j < MAX_N_ELEVATORS; j++){
		for (int i; i < N_FLOORS; i++){
			commands[j][i].timeRegistered = 0;
			commands[j][i].registered = false;	// strengt tatt trenger vi ikke 
			commands[j][i].targeted = false;
		}

		targets[j] = -1;

		activeElevators[j].emergencyStop = false;
		activeElevators[j].registered = false;
		activeElevators[j].priority = NONE;
	}
}


int
orderlist_register_elev (void) {
    int i;

	for (i = 0; i < MAX_N_ELEVATORS; i++) {
		if (activeElevators[i].registered == false) {

			activeElevators[i].registered = true;
			activeElevators[i].emergencyStop = false;
			activeElevators[i].priority = NONE;

			break;
		}
	}

	return i;
}

void
orderlist_delete_elev (int elevator) {
	activeElevators[elevator].emergencyStop = false;
	activeElevators[elevator].registered = false;
	activeElevators[elevator].priority = NONE;

	for (int i=0; i<N_FLOORS; i++){
		commands[elevator][i].timeRegistered=0;
		commands[elevator][i].targeted=0;
		commands[elevator][i].registered=0;
	}
}

// dir bestemmer om callUp eller CalDown skal slettes,
void
orderlist_clear_targeteted_order (int target, int elevator, dir_t elevatorDir) {
	// finnes ikke i etasje N_FLOORS - 1;
	if (target < (N_FLOORS - 1) && elevatorDir) {
		callUp[target].timeRegistered = 0;
		callUp[target].targeted = 0;
		callUp[target].registered = 0;

    // finnes ikke i etasje  0
	} else if (target > 0 && !elevatorDir) {
		callDown[target].timeRegistered = 0;
		callDown[target].targeted = 0;
		callDown[target].registered = 0;
	}

	commands[elevator][target].timeRegistered=0;
	commands[elevator][target].targeted=0;
	commands[elevator][target].registered=0;

	targets[elevator] = -1;
}; 

int
orderlist_register_call_up (int button) {
	if(!callUp[button].registered) {
		callUp[button].timeRegistered = time(NULL);
		callUp[button].registered = true;
		callUp[button].targeted = false;

		return 1;
	}

	return 0;
}

int
orderlist_register_call_down (int button) {
	if(!callDown[button].registered) {
		callDown[button].timeRegistered = time(NULL);
		callDown[button].registered = true;
		callDown[button].targeted = false;

		return 1;
	}

	return 0;
}

int
orderlist_register_call (int floor, int elevator) {
	if(!commands[elevator][floor].registered) {
		commands[elevator][floor].timeRegistered = time(NULL);
		commands[elevator][floor].registered = true;
		commands[elevator][floor].targeted = false;

		return 1;
	}

	return 0;
}

// jeg beholder denne fordi callDown og callUpp kan bli satt av andre heiser
void
orderlist_set_lights (double floor, int elevator) {
	for (int i = 0; i < (N_FLOORS -1); i++){
        elev_set_button_lamp(ELEV_DIR_UP, i, callUp[i].registered );
        elev_set_button_lamp(ELEV_DIR_DOWN, i + 1, callDown[i].registered );
        elev_set_button_lamp(ELEV_DIR_COMMAND, i, commands[elevator][i].registered );
	}

	elev_set_button_lamp(ELEV_DIR_COMMAND, (N_FLOORS - 1), commands[elevator][N_FLOORS - 1].registered );
}

int
orderlist_find_closest_command_under (double floor, int elevator) {
	for (int i = floor; i >= 0; i--) {
		if (commands[elevator][i].registered) {
			return i;
		}
	}

	return -1;	//list is empty
}
int
orderlist_find_closest_command_above (double floor, int elevator) {
	for (int i = floor; i < N_FLOORS; i++) {
		if (commands[elevator][i].registered)
			return i;
	}

	return -1;	//list is empty
}

int
orderlist_find_closest_up_call_under (double floor) {
	for (int i = floor; i >= 0; i--){
		if (i != N_FLOORS -1){		//list only goes to N_FLOORS - 2
			if ( (callUp[i].registered == true) && (callUp[i].targeted == false))
				return i;
		}
	}

	return -1;	//list is empty
}

int 
orderlist_find_closest_up_call_above (double floor) {
	for (int i = floor + 0.5; i < (N_FLOORS - 1); i++){
		if ( (callUp[i].registered == true) && (callUp[i].targeted == false) )
			return i;
	}
	return -1;	//list is empty
}

int
orderlist_find_closest_down_call_under (double floor) {
	for (int i = floor; i > 0; i--){			//list only goes to N_FLOORS - 2
			if ( (callDown[i - 1].registered == true) && (callDown[i - 1].targeted == true) )
				return i; 						//becouse 0 = floor 1 in other lists
	}
	return -1;	//list is empty
}

int
orderlist_find_closest_down_call_above (double floor) {
	for (int i = floor; i < N_FLOORS - 1; i++){
		if ( (callDown[i].registered == true) && (callDown[i].targeted == true) )
			return i + 1;						//becouse 0 = floor 1 in other lists
	}
	return -1;	//list is empty
}

int
orderlist_find_closest_call_above (double floor) {
	int temp = orderlist_find_closest_down_call_above(floor);
	int temp2 = orderlist_find_closest_up_call_above(floor);
	
	if(temp == -1){
		return temp2;
	}else if(temp2 == -1){
		return temp;
	}
	
	if (abs(floor - temp) < abs(floor - temp2)){
		return temp;
	}else{
		return temp2;
	}
}

int
orderlist_find_closest_call_under (double floor) {
	int temp = orderlist_find_closest_down_call_under(floor);
	int temp2 = orderlist_find_closest_up_call_under(floor);
	
	if(temp == -1){
		return temp2;
	}else if(temp2 == -1){
		return temp;
	}
	
	if (abs(floor - temp) < abs(floor - temp2)){
		return temp;
	}else{
		return temp2;
	}
}
