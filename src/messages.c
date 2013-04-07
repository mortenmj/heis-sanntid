#include "messages.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cJSON/cJSON.h>

char*
message_create (message_t msg) {
    char* out;
    cJSON *root, *call_up, *call_down, *cmnds, *targets;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "call_up", call_up=cJSON_CreateObject());
    cJSON_AddItemToObject(root, "call_down", call_down=cJSON_CreateObject());
    cJSON_AddItemToObject(root, "commands", cmnds=cJSON_CreateObject());
    cJSON_AddItemToObject(root, "targets", targets=cJSON_CreateObject());

	for (int i = 0; i < (N_FLOORS-1) ; i++) {
        cJSON *floor;

        cJSON_AddItemToObject(call_up, "floor", floor=cJSON_CreateObject());
        cJSON_AddNumberToObject(floor, "number", i);
        cJSON_AddNumberToObject(floor, "time", callUp[i].timeRegistered);
        cJSON_AddNumberToObject(floor, "registered", callUp[i].registered);
        cJSON_AddNumberToObject(floor, "targeted", callUp[i].targeted);

        cJSON_AddItemToObject(call_down, "floor", floor=cJSON_CreateObject());
        cJSON_AddNumberToObject(floor, "number", i);
        cJSON_AddNumberToObject(floor, "time", callDown[i].timeRegistered);
        cJSON_AddNumberToObject(floor, "registered", callDown[i].registered);
        cJSON_AddNumberToObject(floor, "targeted", callDown[i].targeted);
    }

	for (int i = 0; i < MAX_N_ELEVATORS; i++) {
        cJSON *elev;
        cJSON_AddItemToObject(cmnds, "elevator", elev=cJSON_CreateObject());
        cJSON_AddNumberToObject(elev, "number", i);

		for (int j = 0; j < N_FLOORS; j++) {
            cJSON *floor;

            cJSON_AddItemToObject(elev, "elevator", floor=cJSON_CreateObject());
            cJSON_AddNumberToObject(floor, "number", i);
            cJSON_AddNumberToObject(floor, "time", commands[i][j].timeRegistered);
            cJSON_AddNumberToObject(floor, "targeted", commands[i][j].targeted);
        }
    }

    cJSON_AddNumberToObject(root, "timestamp", time(NULL));
  
    out = cJSON_Print(root);
    cJSON_Delete(root);
      
    return out;
}

message_t
message_parse (char* msg) {
    message_t message;
    char* tmp = malloc(strlen(msg));
    strcpy(tmp, msg); 

    cJSON* root = cJSON_Parse(tmp);

    if (root != 0) {
        message.type = cJSON_GetObjectItem(root, "order_type")->valueint;
        message.floor = cJSON_GetObjectItem(root, "order_floor")->valueint;
        message.timestamp = cJSON_GetObjectItem(root, "order_floor")->valueint;
    }

    free(tmp);
    cJSON_Delete(root);

    return message;
}

void
message_print (message_t msg) {
    printf("type: %d, order floor %d\n", msg.type, msg.floor);
}
