#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <libheis/elev.h>

#include "comms.h"
#include "messages.h"
#include "target.h"

#include "messages.h"
#include "orderlist.h"

#define N_ELEVATORS 4

order_t call_up[N_FLOORS-1];
order_t call_up_local[N_FLOORS-1];
order_t call_down[N_FLOORS-1];
order_t call_down_local[N_FLOORS-1];

elevator_t local_elevator;
elevator_t remote_elevators[N_ELEVATORS];

pthread_mutex_t mutex_call;

static void orderlist_call_up_cb (int button, int value);
static void orderlist_call_down_cb (int button, int value);

/* Private functions */
static void orderlist_call_up_cb (int button, int value) {
    orderlist_set_local_order(button, ORDER_UP, true);
}

static void orderlist_call_down_cb (int button, int value) {
    orderlist_set_local_order(button, ORDER_DOWN, true);
}

/* Public functions */

void
orderlist_init (void) {
    for (int i = 0; i < N_FLOORS-1; i++) {
        call_up[i].set = false;
        call_up[i].updated = false;
  
        call_up_local[i].set = false;
        call_up_local[i].updated = false;
  
        call_down[i].set = false;
        call_down[i].updated = false;

        call_down_local[i].set = false;
        call_down_local[i].updated = false;
  
        elev_register_callback(SIGNAL_TYPE_CALL_UP, &orderlist_call_up_cb);
        elev_register_callback(SIGNAL_TYPE_CALL_DOWN, &orderlist_call_down_cb);
    }
}

void
orderlist_sync (void) {
    /* This should only run if we are master */
    if (!local_elevator.master) {
        return;
    }
}

void
orderlist_update (message_t* m) {
    return;
}

order_t
orderlist_get_local_order (int floor, order_type_t type) {
    pthread_mutex_lock(&mutex_call);
    switch(type) {
      case ORDER_UP:
        return call_up_local[floor];
        break;
      case ORDER_DOWN:
        return call_down_local[floor];
        break;
    }
    pthread_mutex_unlock(&mutex_call);
}

void
orderlist_set_local_order (int floor, order_type_t type, bool value) {
    pthread_mutex_lock(&mutex_call);
    switch(type) {
      case ORDER_UP:
        call_up_local[floor].set = value;
        call_up_local[floor].updated = true;
        break;
      case ORDER_DOWN:
        call_down_local[floor].set = value;
        call_down_local[floor].updated = true;
    }
    pthread_mutex_unlock(&mutex_call);
}

elevator_t
orderlist_get_local_elevator (void) {
    return local_elevator;
}

elevator_t*
orderlist_get_remote_elevators (void) {
    return remote_elevators;
}

void
orderlist_set_master_status (bool status) {
    local_elevator.master = status;
}

bool
orderlist_get_master_status (void) {
    return local_elevator.master;
}
