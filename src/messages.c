#include "messages.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cJSON/cJSON.h>

#include "comms.h"
#include "orderlist.h"

char*
message_create_status (void) {
    char* out;
    cJSON *root, *status, *call_up, *call_down, *floor;
    elevator_t s = orderlist_get_local_elevator();

    printf("creating message\n");
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "sender", inet_ntoa(comms_get_address()));

    cJSON_AddItemToObject(root, "status", status=cJSON_CreateObject());
    cJSON_AddNumberToObject(status, "master", s.master);
    cJSON_AddNumberToObject(status, "priority", s.priority);
    cJSON_AddNumberToObject(status, "emergency_stop", s.emergency_stop);
    cJSON_AddNumberToObject(status, "floor", s.floor);
    
    cJSON_AddItemToObject(root, "call_up", call_up=cJSON_CreateArray());
    cJSON_AddItemToObject(root, "call_down", call_down=cJSON_CreateArray());

    printf("entering loop\n");
	for (int i = 0; i < (N_FLOORS-1) ; i++) {
        printf("adding up orders\n");
        cJSON_AddItemToArray(call_up, floor=cJSON_CreateObject());
        order = orderlist_get_local_order(i, ORDER_UP);
        cJSON_AddNumberToObject(floor, "set", order.set);
        cJSON_AddNumberToObject(floor, "updated", order.updated);

        printf("adding down orders\n");
        order = orderlist_get_local_order(i, ORDER_DOWN);
        printf(".\n");
        cJSON_AddItemToObject(call_down, "floor", floor=cJSON_CreateObject());
        printf(".\n");
        cJSON_AddNumberToObject(floor, "set", order.set);
        printf(".\n");
        cJSON_AddNumberToObject(floor, "updated", order.updated);
        printf("added down orders\n");
    }
    printf("left loop\n");

    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("message create\n");

    return out;
}

int
message_parse_status (message_t* m, char* msg) {
    cJSON *root, *status, *up, *down, *floor;
    elevator_t e;
    char* senderstr;
    char* tmp = malloc(strlen(msg));
    strcpy(tmp, msg); 

    root = cJSON_Parse(tmp);

    if (root == 0) {
        return 1;
    }

    m->sender = inet_addr(cJSON_GetObjectItem(root, "sender")->valuestring);

    status = cJSON_GetObjectItem(root, "status");
    e.master = cJSON_GetObjectItem(status, "master")->valueint;
    e.priority = cJSON_GetObjectItem(status, "priority")->valueint;
    e.emergency_stop = cJSON_GetObjectItem(status, "emergency_stop")->valueint;
    e.floor = cJSON_GetObjectItem(status, "floor")->valueint;
    m->elevator = e;

    up = cJSON_GetObjectItem(root, "call_up");
    down = cJSON_GetObjectItem(root, "call_down");

    for (int i = 0; i < N_FLOORS-1; i++) {
        floor = cJSON_GetArrayItem(up, i);
        m->call_up[i].set = cJSON_GetObjectItem(floor, "set")->valueint;
        m->call_up[i].updated = cJSON_GetObjectItem(floor, "updated")->valueint;

        floor = cJSON_GetArrayItem(down, i);
        m->call_down[i].set = cJSON_GetObjectItem(floor, "set")->valueint;
        m->call_down[i].updated = cJSON_GetObjectItem(floor, "updated")->valueint;
    }

    free(tmp);
    cJSON_Delete(root);

    return 0;
}
