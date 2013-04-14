#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <libheis/elev.h>

#include "target.h"
#include "comms.h"
#include "messages.h"
#include "orderlist.h"

int target = -1;
int locked_target = -1;
target_type_t target_type;

//------------ UPDATE TARGET ------------

static int target_find_closest_command_under (double floor);
static int target_find_closest_command_above (double floor);
static int target_find_closest_call_up_under (double floor);
static int target_find_closest_call_up_above (double floor);
static int target_find_closest_call_down_under (double floor);
static int target_find_closest_call_down_above (double floor);
static int target_find_closest_call_above (double floor, rmode_t rmode);
static int target_find_closest_call_under (double floor, rmode_t rmode);
static int target_find_closest_call_down (double floor, rmode_t rmode);
static int target_find_closest_of (int temp_command, int temp_call, priority_t mode, rmode_t rmode);
static int target_find_priority_or_target (int temp_target_above, int temp_target_under, double floor, int return_mode);
static int target_value_upward_or_downward_elev (double floor, int value);

/* Private functions */

static int
target_find_closest_command_under (double floor) {
    elevator_t local = orderlist_get_local_elevator();

	for (int i = floor; i >= 0; i--) {
		if (local.commands[i].set == true) {
			return i;
		}
	}
	return -1;	//list is empty
}

static int
target_find_closest_command_above (double floor) {
    elevator_t local = orderlist_get_local_elevator();

	for(int i = floor; i < N_FLOORS; i++) {
		if (local.commands[i].set == true)
			return i;
	}
	return -1;	//list is empty
}

static int
target_find_closest_call_up_under (double floor) {
	for (int i = floor; i >= 0; i--) {
		if (i != N_FLOORS -1) {		//list only goes to N_FLOORS - 2
			if (orderlist_get_local_order(i, ORDER_UP).set == true)
				return i;
		}
	}
	return -1;	//list is empty
}

static int
target_find_closest_call_up_above (double floor) { 
	for (int i = ceil(floor); i < (N_FLOORS - 1) ; i++) {
		if (orderlist_get_local_order(i, ORDER_UP).set == true)
			return i;
	}
	return -1;	//list is empty
}

static int
target_find_closest_call_down_under (double floor) {
	for (int i = floor; i > 0; i--) {			//list only goes to N_FLOORS - 2
			if (orderlist_get_local_order(i-1, ORDER_DOWN).set == true)
				return i; 						//becouse 0 = floor 1 in other lists
	}
	return -1;	//list is empty
}

static int
target_find_closest_call_down_above (double floor) {
	if(floor < 1)
		floor = 1;

	for (int i = floor; i < N_FLOORS ; i++) {
		if (orderlist_get_local_order(i-1, ORDER_DOWN).set == true)
			return i;						//becouse 0 = floor 1 in other lists
	}
	return -1;	//list is empty
}

static int
target_find_closest_call_above (double floor, rmode_t rmode) {
	int down = target_find_closest_call_down_above (floor);
	int up = target_find_closest_call_up_above (floor);
	

	if(down == -1) {
	
	    if(rmode == TARGET)
		    return up;
		if(rmode == ORDERTYPE)
		    return TYPE_CALL_UP;
		    
	} else if(up == -1) {
	
	    if(rmode == TARGET)
		    return down;
		if(rmode == ORDERTYPE)
		    return TYPE_CALL_DOWN;
	}
	if (abs(floor - down) < abs(floor - up)) {
        if(rmode == TARGET)
		    return down;
		if(rmode == ORDERTYPE)
		    return TYPE_CALL_DOWN;
	} else {
	    if(rmode == TARGET)
		    return up;
		if(rmode == ORDERTYPE)
		    return TYPE_CALL_UP;
	}
}

static int
target_find_closest_call_under (double floor, rmode_t rmode) {
	int down = target_find_closest_call_down_under(floor);
	int up = target_find_closest_call_up_under(floor);

	if(down == -1){
		if(rmode == TARGET)
		    return up;
		if (rmode == ORDERTYPE)
		    return TYPE_CALL_UP;

	}else if(up == -1){
		if(rmode == TARGET)
		    return down;
		if (rmode == ORDERTYPE)
		    return TYPE_CALL_DOWN;
	}
	
	if (abs(floor - down) < abs(floor - up)) {
		if(rmode == TARGET)
		    return down;
		if (rmode == ORDERTYPE)
		    return TYPE_CALL_DOWN;
	} else if (abs(floor - down) > abs(floor - up)) {
	    if (rmode == TARGET)
	        return up;
	    if(rmode == ORDERTYPE)
	        return TYPE_CALL_UP;
	} else {
	    if (rmode == TARGET)
	        return up;
	    if (rmode == ORDERTYPE)
	        return TYPE_CALL_UP;
    }
}

/* Mode: upp = closest is the smalest, down = closest is the biggest. Return_mode: 1 = return target, 2 = return 1 if temp1 was closest, 2 if temp2 was closest, 0 if they where just as close */
static int
target_find_closest_of (int temp1, int temp2, priority_t mode, rmode_t rmode) {
    if(!( temp2 == -1 || temp1 == -1)) {
				switch(mode) {
					case 1:
						if (temp2 <= temp1) {
							if(rmode == 1)
							    return temp2;
							if(rmode == 2)
							    return 2;

						} else if (temp2 > temp1) {
						    if(rmode == 1)
							    return temp1;
							if(rmode == 2)
							    return 1;

						}
					break;

					case 2:
						if (temp2 >= temp1) {
						    if(rmode == 1)
							    return temp2;
							if(rmode == 2)
							    return 2;

						} else if (temp2 < temp1) {
						    if(rmode == 1)
							    return temp1;
							if(rmode == 2)
							    return 1;

						}
					break;
				}
		}

		if ( temp2 != -1 && temp1 == -1 ) {
			if(rmode == 1)    
			    return temp2;
			if(rmode == 2)
			    return 2;
		}
		if ( temp2 == -1 && temp1 != -1 ) {
		    if(rmode == 1)
			    return temp1;
			if(rmode == 2)
			    return 1;
		}
		
		if(rmode == TARGET)
		    return target;
		if(rmode == ORDERTYPE)
		    return NONE;
}

static int 
target_find_priority_or_target (int temp_target_above, int temp_target_under, double floor, int return_mode) {	//returns new Target if returnMode = 1, returns Priority if returnMode = 0
	int temp_target = -1;
	int temp_priority = NONE;
	if( (temp_target_above == -1) ^ (temp_target_under == -1) ) { 	//if one of the lists are emptry

		if (temp_target_above == -1){
			temp_target = temp_target_under;
			temp_priority = DOWNWARD; 				//DOWNWARD;
		}else{								//targetUnder has to be empty
			temp_target = temp_target_above;
			temp_priority = UPWARD; 				//UPWARD;
		}
	
	}else if ( (temp_target_above != -1) && (temp_target_under != -1) ){				//if there are targets in both lists.

		if ( abs(floor - temp_target_above) == abs(floor - temp_target_under)){ 		//if distance is equal:

			if(floor == (double)(N_FLOORS - 1) / (double)2 ){										//if elevator is in center, down is temp_priority ((N_FLOORS - 1) / 2) = 1.5
				temp_target = temp_target_under;
				temp_priority = DOWNWARD;		
			}else if (floor > ((N_FLOORS - 1) / 2)){		//if temp_target above is closest to edge
				temp_target = temp_target_above;
				temp_priority = UPWARD;
			}else{
				temp_target = temp_target_under;
				temp_priority = DOWNWARD;
			}

		}else if (abs(floor - temp_target_above) > abs(floor - temp_target_under)){
			temp_target = temp_target_under;
			temp_priority = 2;	
		}else{									
			temp_target = temp_target_above;
			temp_priority = UPWARD;	
		}
	}
	if(return_mode == 1){
		return temp_target;
	}else if(return_mode == 0){
		return temp_priority;
	}
	return 0;
}

static int target_value_upward_or_downward_elev (double floor, int value) { if(floor > value){
        return DOWNWARD;
    } else if (floor < value){
        return UPWARD;
    } else {
        return EQUAL;
    }
}

/* Public functions */

int target_update (double floor) {
	int temp_target_above;
	int temp_target_under;
	int temp_command;
	int temp_call;

    elevator_t local = orderlist_get_local_elevator();

	switch (local.priority) {
		
		case LOCKONOUTSIDE:
		    temp_target_above = target_find_closest_call_above(floor, TARGET);
			temp_target_under = target_find_closest_call_under(floor, TARGET);
			
			if(locked_target == -1){
			    locked_target = target_find_priority_or_target(temp_target_above , temp_target_under, floor, TARGET);
				target = locked_target;
			}
			
			if(target_value_upward_or_downward_elev(floor, target) == UPWARD){
			    target_type = TYPE_CALL_DOWN;
		    } else if (target_value_upward_or_downward_elev(floor, locked_target) == DOWNWARD){
			    target_type = TYPE_CALL_UP;
			}

			if(target == -1) {
				local.priority = NONE;
				locked_target = -1;
			}

		break;

		case UPWARD:
		    target_type = TYPE_CALL_UP;
			temp_command = target_find_closest_command_above (floor);
			temp_call = target_find_closest_call_up_above (floor);

			target = target_find_closest_of (temp_command,temp_call, UPWARD, TARGET);

			if((temp_call == -1) && (temp_command == -1)) {
				local.priority=NONE;	
            }
		break;
		
		case DOWNWARD:
		    target_type = TYPE_CALL_DOWN;
			temp_call = target_find_closest_command_under(floor);
			temp_command = target_find_closest_call_down_under(floor);

			target = target_find_closest_of (temp_command,temp_call, DOWNWARD, TARGET);
			
			
			if((temp_call == -1) && (temp_command == -1)) {
				local.priority=NONE;
            }
		break;
		
		case NONE:
			temp_target_above = target_find_closest_command_above (floor);
			temp_target_under = target_find_closest_command_under (floor);
			local.priority = target_find_priority_or_target (temp_target_above, temp_target_under, floor, PRIORITY);
			
			if(temp_target_above == -1 && temp_target_under == -1 && (target_find_closest_call_above (floor , TARGET) != -1 || target_find_closest_call_under (floor , TARGET) != -1)) {
		        temp_target_above = target_find_closest_call_above(floor, TARGET);
			    temp_target_under = target_find_closest_call_under(floor, TARGET);
                
                if(temp_target_above == temp_target_under && temp_target_above == floor ){  // if call is on the same floor as the elevator
                	if (target_find_closest_call_above(floor, ORDERTYPE) == TYPE_CALL_UP){
                		local.priority = UPWARD;
                		break;
                	}
                	if (target_find_closest_call_above(floor, ORDERTYPE) == TYPE_CALL_DOWN){
                		local.priority = DOWNWARD;
                		break;
                	}
                }
                
			    local.priority = target_find_priority_or_target(temp_target_above , temp_target_under, floor, PRIORITY);
                
                // when i have a callUp above me or a callDown under me
			    if((local.priority == UPWARD &&
                      target_find_closest_call_above(floor, ORDERTYPE) == TYPE_CALL_UP) || (local.priority == DOWNWARD && target_find_closest_call_under(floor, ORDERTYPE) == TYPE_CALL_DOWN)) {
			        break;
			    }

			    local.priority = LOCKONOUTSIDE;
			    
			}
			//target = -1; // strengt tatt er denne ikke nødvendig
			
			break;
	}

	return target;
	}

void
target_clear_completed_order (void) {
    elevator_t local = orderlist_get_local_elevator();

	if (target < (N_FLOORS - 1) && target_type == TYPE_CALL_UP ) {
        orderlist_set_local_order(target, ORDER_UP, false);
		
	} else if (target > 0 && target_type == TYPE_CALL_DOWN) {
        orderlist_set_local_order(target-1, ORDER_DOWN, false);
    }

	local.commands[target].set = false;
    target = -1;
}
