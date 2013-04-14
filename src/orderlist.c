#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <libheis/elev.h>

#include "comms.h"
#include "messages.h"
#include "orderlist.h"
#include "target.h"
#include "types.h"

int my_id;

order_t callUp[N_FLOORS-1];					    // 0 = floor 1, N_FLOORS = floor N_FLOORS - 1.
order_t callDown[N_FLOORS-1];					    // 0 = floor 2, N_FLOORS = floor N_FLOORS.
elevator_t elevators[MAX_N_ELEVATORS];

elevator_t local_elevator;

static bool callUp_mutexed[N_FLOORS-1];				// 0 = floor 1, N_FLOORS = floor N_FLOORS - 1.
static bool callDown_mutexed[N_FLOORS-1];				// 0 = floor 2, N_FLOORS = floor N_FLOORS.
static bool commands_mutexed[N_FLOORS];

pthread_mutex_t mutex_call;
pthread_mutex_t mutex_callUp;
pthread_mutex_t	mutex_callDown;
pthread_mutex_t mutex_commands;

static void orderlist_callup_cb (int button, int dummy);
static void orderlist_calldown_cb (int floor, int dummy);
static void orderlist_command_cb (int button, int dummy);

static int orderlist_find_elevator (struct in_addr addr);
static int orderlist_find_ip_less_than (struct in_addr addr, priority_t priority);
static int orderlist_update_remote_elev (struct in_addr addr, elevator_t* s);
static void orderlist_sync_external_orders (message_t *message);
static void orderlist_update_synced_status (message_t *m);
static bool orderlist_is_local_elev_synced (void);
static bool orderlist_global_sync (void);
static int orderlist_reset_synced_list (void);
static int orderlist_add_elev_ready_to_sync(struct in_addr ext_elev_ready_for_sync);
static void orderlist_read_commands ();
void orderlist_commands_to_file ();

//=====================================================PRIVATE FUNCTIONS==================================================

static void 
orderlist_callup_cb (int button, int dummy) {
	pthread_mutex_lock(&mutex_callUp);
	callUp_mutexed[button] = true;
	pthread_mutex_unlock(&mutex_callUp);
}

static void
orderlist_calldown_cb (int floor, int dummy) {
	pthread_mutex_lock(&mutex_callDown);
	callDown_mutexed[floor-1] = true;
	pthread_mutex_unlock(&mutex_callDown);
}

static void
orderlist_command_cb (int button, int dummy) {
	pthread_mutex_lock(&mutex_commands);
	commands_mutexed[button] = true;
	pthread_mutex_unlock(&mutex_commands);
}

static int
orderlist_find_elevator (struct in_addr addr) {
    for ( int elevator=0; elevator < MAX_N_ELEVATORS ; elevator++) {
        if (elevators[elevator].addr.s_addr == addr.s_addr) {
            return elevator;
        }
    }
    return -1;
}

static int
orderlist_find_ip_less_than (struct in_addr addr, priority_t priority) {
    for ( int elevator=0; elevator < MAX_N_ELEVATORS ; elevator++) {
        if (elevators[elevator].addr.s_addr < addr.s_addr && elevators[elevator].priority == priority) {
            return true;
        }
    }
    return false; 
}

static int
orderlist_update_remote_elev (struct in_addr addr, elevator_t* s) {
    // finn heisen i elevators
    // om vi kommer til en heis med addresse -1 er den slettet;
    // om ikke funnet, legg til sist
    // oppdater informasjon
    // return true if a new elevator was added
    // return false if i uppdated or nothing was done

    /* Find the elevator we're looking for, if we did not find it look for a empty spot to store the new elevator */
	for (int i = 0; i < MAX_N_ELEVATORS; i++) {
        if (elevators[i].addr.s_addr == addr.s_addr) {
            elevators[i].priority = s->priority;
            elevators[i].emergency_stop = s->emergency_stop;
            elevators[i].floor = s->floor;

            return 0;
        }
    }

   	for (int i = 0; i < MAX_N_ELEVATORS; i++) {
        if ((unsigned long)elevators[i].addr.s_addr == 0) {
            elevators[i].priority = s->priority;
            elevators[i].emergency_stop = s->emergency_stop;
            elevators[i].floor = s->floor;
            elevators[i].global_sync = s->global_sync;
            elevators[i].addr = addr;

            return 0;
        }
    }
    
    return 1;
}

static void
orderlist_sync_external_orders (message_t *m) {
    /* Update elevator data */
    struct in_addr addr;
    addr.s_addr = m->sender;
    orderlist_update_remote_elev (addr, &(m->status));


    for (int i = 0; i < N_FLOORS-1 ; i++) {
    /*
		if ( (!m->callUp[i].registered && m->callUp[i].updated) && !(callUp[i].registered && callUp[i].updated) ) {
			if (!(callUp[i].registered && callUp[i].updated)) {
				callUp[i].registered = false;
				callUp[i].updated = true;
			}
		} else if ( !(!callUp[i].registered && callUp[i].updated) ) {
			callUp[i].registered = m->callUp[i].registered || callUp[i].registered;
			callUp[i].updated = m->callUp[i].updated || callUp[i].updated;
		}

		if ((!m->callDown[i].registered && m->callDown[i].updated) && !(callDown[i].registered && callDown[i].updated)) {
			callDown[i].registered = false;
			callUp[i].updated = true;
		} else if (!(!callDown[i].registered && callDown[i].updated)){
			callDown[i].registered = m->callDown[i].registered || callDown[i].registered;
			callDown[i].updated = m->callDown[i].updated || callDown[i].updated;
		}
    }
    */
			
        // If the remote elevator has a new order we lack 
        if (m->callUp[i].registered && m->callUp[i].updated) {
            if (!callUp[i].registered && !callUp[i].updated) {
                callUp[i].registered = true;
                callUp[i].updated = true;
            }
        // If the remote elevator has newly deleted an order we still have 
        } else if (!m->callUp[i].registered && m->callUp[i].updated) {
            if (callUp[i].registered && !callUp[i].updated) {
                callUp[i].registered = false;
                callUp[i].updated = true;
            }
        }

        // DOWN 
        // If the remote elevator has a new order we lack 
        if (m->callDown[i].registered && m->callDown[i].updated) {
            if (!callDown[i].registered && !callDown[i].updated) {
              callDown[i].registered = true;
              callDown[i].updated = true;
            }
        // If the remote elevator has newly deleted an order we still have 
        } else if (!m->callDown[i].registered && m->callDown[i].updated) {
            if (callDown[i].registered && !callDown[i].updated) {
                callDown[i].registered = false;
                callDown[i].updated = true;
            }
        }
	}
}

static void
orderlist_update_synced_status (message_t *m) {
	bool synced = true;
	for (int i = 0; i < MAX_N_ELEVATORS; i++) {
		if (elevators[i].addr.s_addr == m->sender) {
            for (int j = 0; j < N_FLOORS-1; j++) {
                if (callUp[j].registered != m->callUp[j].registered || callUp[j].updated != m->callUp[j].updated) {
                    synced = false;
                }
                if (callDown[j].registered != m->callDown[j].registered || callDown[j].updated != m->callDown[j].updated) {
                    synced = false;
                }
            }

            elevators[i].synced = synced;
            return;
        }
    }
}

static bool
orderlist_is_local_elev_synced (void) {
	for (int i = 0; i < MAX_N_ELEVATORS; i++) {
		if (elevators[i].addr.s_addr != 0 && elevators[i].synced == false) {
            local_elevator.synced = false;
		}
	}
    return true;
}

static bool
orderlist_global_sync (void) {
    for (int i = 0; i < MAX_N_ELEVATORS; i++) {
        if (elevators[i].addr.s_addr != 0 && elevators[i].global_sync == false) {
            return false;
        }
    }

    return true;
}

static int
orderlist_reset_synced_list (void) {
	for (int i=0; i < MAX_N_ELEVATORS; i++) {
		elevators[i].synced = false;
	}
}

static int
orderlist_add_elev_ready_to_sync(struct in_addr ext_elev_ready_for_sync){
	for(int i = 0; i < MAX_N_ELEVATORS; i++){
		if (ext_elev_ready_for_sync.s_addr == elevators[i].elev_ready_for_sync.s_addr){
			return 0;
		}
	}
	for(int i = 0; i < MAX_N_ELEVATORS; i++){
		if (elevators[i].elev_ready_for_sync.s_addr == 0){
			elevators[i].elev_ready_for_sync.s_addr = ext_elev_ready_for_sync.s_addr;
			return 1;
		}
	}
	return 0;
}

static void
orderlist_read_commands () {
	int tempcom[N_FLOORS];
	
	FILE *frp = fopen("commands.dat", "r");
	if (frp == NULL){
		printf("read_commands: failed to open commands.dat\n");
		return;
	}
	for(int i=0; i < N_FLOORS; i++){
		if( fscanf(frp, "%d\n", &tempcom[i]) != 1 ){
			printf("read_commands: failure during scan of document\n");
			fclose(frp);
		}
	}
	for(int i=0; i < N_FLOORS;i++){
		if(tempcom[i] == false || tempcom[i] == true){	
			local_elevator.commands[i].registered = (bool)tempcom[i];
		} else {
			printf("read_commands: data in wrong format\n");
		}
	}
}

void
orderlist_commands_to_file () {
	FILE *fwp = fopen("commands.dat", "w");

	if (fwp == NULL){
		printf("commands_to_file: fopen failed to open create commands.dat\n");
		exit(1);
	} else {
		for (int i=0; i < N_FLOORS; i++) {
			fprintf(fwp, "%d\n", (int)local_elevator.commands[i].registered);
		}
		fclose(fwp);
	}
}

/* Public functions */

void
orderlist_register_local_orders (void) {
	pthread_mutex_lock(&mutex_commands);
	
	for (int i = 0; i < N_FLOORS; i++) {
		if (commands_mutexed[i]) {
			local_elevator.commands[i].registered = true;
			orderlist_commands_to_file ();
			commands_mutexed[i] = false;
		}
	}
	
	pthread_mutex_unlock(&mutex_commands);

	pthread_mutex_lock(&mutex_callDown);
	for (int i = 0; i < N_FLOORS-1; i++) {
		if (callDown_mutexed[i]) {
		    if (!callDown[i].registered) {              
			    callDown[i].updated = true;
	  	    }
			
			callDown[i].registered = true;
			callDown_mutexed[i] = false;
		}
	}
	pthread_mutex_unlock(&mutex_callDown);


	pthread_mutex_lock(&mutex_callUp);
	for (int i = 0; i < N_FLOORS-1; i++) {
		if (callUp_mutexed[i]) {
			if (!callUp[i].registered) {
				callUp[i].updated = true;
			}

			callUp[i].registered = true;
			callUp_mutexed[i] = false;
		}
	}
	pthread_mutex_unlock(&mutex_callUp);
}

void orderlist_clear_update_flag (void) {
    for (int i = 0; i < N_FLOORS-1 ; i++) {
        callUp[i].updated = false;
        callDown[i].updated = false;
    }
}

void
orderlist_set_lights (void) {
	for (int i = 0; i < (N_FLOORS -1); i++){
        elev_set_button_lamp(ELEV_DIR_UP, i, callUp[i].registered );
        elev_set_button_lamp(ELEV_DIR_DOWN, i + 1, callDown[i].registered );
        elev_set_button_lamp(ELEV_DIR_COMMAND, i, local_elevator.commands[i].registered );
	}

	elev_set_button_lamp(ELEV_DIR_COMMAND, (N_FLOORS - 1), local_elevator.commands[N_FLOORS - 1].registered );
}

int
orderlist_init (void) {
	for (int i=0; i < (N_FLOORS-1) ; i++){
		callUp[i].registered = false;
		callUp[i].updated = false;
		callUp_mutexed[i] = false;

		callDown[i].registered = false;
		callDown[i].updated = false;
		callDown_mutexed[i] = false;

		commands_mutexed[i] = false;
        local_elevator.commands[i].registered = false;
        local_elevator.commands[i].updated = false;
	}
	commands_mutexed[N_FLOORS-1] = false;
	local_elevator.commands[N_FLOORS-1].registered = false;
	local_elevator.commands[N_FLOORS-1].updated = false;
	local_elevator.priority = NONE;
	local_elevator.addr = comms_get_address();
	local_elevator.elev_ready_for_sync.s_addr = 0;
	local_elevator.floor = -1;
	local_elevator.emergency_stop = false;
	local_elevator.synced = false;

	for (int i = 0; i < MAX_N_ELEVATORS; i++){
		for (int j = 0; j < N_FLOORS; j++){
			elevators[i].commands[j].registered = false;
			elevators[i].commands[j].updated = false;
		}

		elevators[i].emergency_stop = false;
		elevators[i].priority = NONE;
		elevators[i].floor = 0;
		elevators[i].addr.s_addr = 0;
		elevators[i].elev_ready_for_sync.s_addr = 0;
		elevators[i].synced = false;


	}
	
	orderlist_read_commands (&(local_elevator.commands));
	
	elev_register_callback( SIGNAL_TYPE_CALL_UP, &orderlist_callup_cb );
	elev_register_callback( SIGNAL_TYPE_CALL_DOWN, &orderlist_calldown_cb );
	elev_register_callback( SIGNAL_TYPE_COMMAND, &orderlist_command_cb );

    return 1;
}

elevator_t*
orderlist_get_local_elevator (void) {
    return &local_elevator;
}

order_t
orderlist_get_local_order (int floor, order_type_t type) {
    order_t* tmp;
	
    pthread_mutex_lock(&mutex_call);
    switch(type) {
      case ORDER_UP:
        tmp = &callUp[floor];
        break;
      case ORDER_DOWN:
        tmp = &callDown[floor];
        break;
    }
    pthread_mutex_unlock(&mutex_call);

    return *tmp;
}

void
orderlist_set_local_order (int floor, order_type_t type, bool value) {
    pthread_mutex_lock(&mutex_call);
    switch(type) {
      case ORDER_UP:
        callUp[floor].registered = value;
        callUp[floor].updated = true;
        break;
      case ORDER_DOWN:
        callDown[floor].registered = value;
        callDown[floor].updated = true;
    }
    pthread_mutex_unlock(&mutex_call);
}

/* Return 0 if orderlist is synchronised successfully */
int
orderlist_sync (int fd) {
	int numbytes;
	char* msg;
	message_t *m = malloc(sizeof(message_t));
	bool synced = false;
	bool not_timeout = true;

    while (!synced) {
        for (int i = 0; i < 2;i++) {
        numbytes = comms_listen(fd, &msg);
        /*
         * når 2 heiser broadcaster på nettverket må det leses to ganger
         * på begge. er ikke helt sikker på hva som sjer. selv om vi
         * ignorer egne beskjeder hjalp dette. Kanskje det hoper seg opp av
         * gamle beskjeder om vi ikke gjør dette. sletting av target funker
         * ikke helt uten for 2 løkke så virker det hvertfall som systemet
         * motar en hel del besjeder med callUp informasjon som er gamle.
         * jo lengre man venter med og trykke på en knapp etter at man har
         * startet systemet jo lengre tid tar det før den andre heisen
         * plukker opp opdateringen syncronisering av slettet target
         * funker ikke helt med koden min	
        */

        
        if (numbytes > 0) {
            msg[numbytes] = '\0'; message_parse_status(m, msg);
        
            struct in_addr addr;
            addr.s_addr = m->sender;
            orderlist_sync_external_orders(m);
            orderlist_update_synced_status(m);
        }

        local_elevator.global_sync = orderlist_is_local_elev_synced();
        synced = orderlist_global_sync();
        }

        msg = message_create_status();
        comms_send_data(msg);
        
        /*
        printf("\033[2J\033[1;1H"); 
        orderlist_print_lists();
        printf("1\n"); printf("synced %d\n",synced);
        */
		
	}
	usleep(1000);
	
	orderlist_reset_synced_list();
    local_elevator.global_sync = false;

	free(m);

	return 0;
}
  
void orderlist_print_lists(void){
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("------------------------------------\n");
	printf("-------The Fabulous Elevator--------\n");
	printf("------------------------------------\n");

	printf("EXTERNAL ELEVATORS: \t");
	printf("\n");
	printf("\tip: \t\t\t");
	for (int i = 0; i < MAX_N_ELEVATORS; i++){
	        printf("%s, ", inet_ntoa(elevators[i].addr));
	}
	printf("\n");
	printf("\tsynced with: \t\t");
	for (int i = 0; i < MAX_N_ELEVATORS; i++){
		printf("%d, ", elevators[i].synced);
	}
	printf("\n");
	printf("LOCAL_ELEVATOR: \t\n");
	printf("\tIP: \t\t\t");
	printf("%s", inet_ntoa(local_elevator.addr));
	printf("\n");
	printf("\tCallUp REGISTERED:\t");
	for (int i = 0; i < N_FLOORS - 1; i++){
		printf("%i, ", callUp[i].registered);
	}
	printf("\n");
	printf("\tCallUp UPDATED:\t\t");
	for (int i = 0; i < N_FLOORS - 1; i++){
		printf("%i, ", callUp[i].updated);
	}

	printf("\n");

	printf("\tCallDown REGISTERED: \t   ");
	for (int i = 0; i < N_FLOORS - 1; i++){
		printf("%i, ", callDown[i].registered);
	}
	printf("\n");
	printf("\tCallDown UPDATED: \t   ");
	for (int i = 0; i < N_FLOORS - 1; i++){
		printf("%i, ", callDown[i].updated);
	}

	printf("\n");
	printf("\tCOMMANDS: \t\t");
	for (int i = 0; i < N_FLOORS; i++){
		printf("%i, ", local_elevator.commands[i].registered);
	}
	printf("\n");
	if(local_elevator.priority == 1)
		printf("\tPriority: \tUPWARD\n");
	else if (local_elevator.priority == 2)
		printf("\tPriority: \tDOWNWARD\n");	
	else if (local_elevator.priority == 0)
		printf("\tPriority: \tNONE\n");	
	else if (local_elevator.priority == 3)
		printf("\tPriority: \tLOCKONOUTSIDE\n");
}
