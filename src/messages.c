#include "messages.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cJSON/cJSON.h>

#include "comms.h"
#include "types.h"

char*
message_create_status (void) {
    char* out;
    cJSON *root, *status, *call_up, *call_down, *floor;
    elevator_t *s = orderlist_get_local_elevator();

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "sender", inet_ntoa(comms_get_address()));

    cJSON_AddItemToObject(root, "status", status=cJSON_CreateObject());
    cJSON_AddNumberToObject(status, "priority", s->priority);
    cJSON_AddNumberToObject(status, "emergency_stop", s->emergency_stop);
    cJSON_AddNumberToObject(status, "floor", s->floor);
    cJSON_AddNumberToObject(status, "synced", s->synced);
    cJSON_AddNumberToObject(status, "global_sync", s->global_sync);
    
    cJSON_AddItemToObject(root, "call_up", call_up=cJSON_CreateArray());
    cJSON_AddItemToObject(root, "call_down", call_down=cJSON_CreateArray());

	for (int i = 0; i < (N_FLOORS-1) ; i++) {
        cJSON_AddItemToArray(call_up, floor=cJSON_CreateObject());
        cJSON_AddNumberToObject(floor, "registered", callUp[i].registered);
        cJSON_AddNumberToObject(floor, "updated", callUp[i].updated);

        cJSON_AddItemToObject(call_down, "floor", floor=cJSON_CreateObject());
        cJSON_AddNumberToObject(floor, "registered", callDown[i].registered);
        cJSON_AddNumberToObject(floor, "updated", callDown[i].updated);
    }

    out = cJSON_Print(root);
    cJSON_Delete(root);

    return out;
}

int
message_parse_status (message_t* m, char* msg) {
    cJSON *root, *status, *up, *down, *floor;
    elevator_t e;
    char* senderstr;
    char* tmp = malloc(strlen(msg));
    strcpy(tmp, msg); 
    order_t callUp_tmp[N_FLOORS-1];

    root = cJSON_Parse(tmp);

    if (root == 0) {
        return 1;
    }

    m->sender = inet_addr(cJSON_GetObjectItem(root, "sender")->valuestring);

    status = cJSON_GetObjectItem(root, "status");
    e.priority = cJSON_GetObjectItem(status, "priority")->valueint;
    e.emergency_stop = cJSON_GetObjectItem(status, "emergency_stop")->valueint;
    e.floor = cJSON_GetObjectItem(status, "floor")->valueint;
    e.synced = cJSON_GetObjectItem(status, "synced")->valueint;
    e.global_sync = cJSON_GetObjectItem(status, "global_sync")->valueint;
    m->status = e;

    up = cJSON_GetObjectItem(root, "call_up");
    down = cJSON_GetObjectItem(root, "call_down");

    for (int i = 0; i < N_FLOORS-1; i++) {
        floor = cJSON_GetArrayItem(up, i);
        m->callUp[i].registered = cJSON_GetObjectItem(floor, "registered")->valueint;
        m->callUp[i].updated = cJSON_GetObjectItem(floor, "updated")->valueint;

        floor = cJSON_GetArrayItem(down, i);
        m->callDown[i].registered = cJSON_GetObjectItem(floor, "registered")->valueint;
        m->callDown[i].updated = cJSON_GetObjectItem(floor, "updated")->valueint;
    }

    free(tmp);
    cJSON_Delete(root);

    return 0;
}
