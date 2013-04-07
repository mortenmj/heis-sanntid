#include "messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <cJSON/cJSON.h>

char* messages_create_order(int floor, int type) {
    char* out;
    cJSON *root, *cli;

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "test", "test");
    cJSON_AddItemToObject(root, "elev_order", cli=cJSON_CreateObject());
    cJSON_AddNumberToObject(cli, "order_type", type);
    cJSON_AddNumberToObject(cli, "order_floor", floor);

    out = cJSON_Print(root);
    cJSON_Delete(root);


    return out;
}

order_t messages_parse_order(char* msg) {
    order_t order;
    char* tmp = malloc(strlen(msg));
    strcpy(tmp, msg); 

    cJSON* root = cJSON_Parse(tmp);

    if (root != 0) {
      cJSON *elev_order = cJSON_GetObjectItem(root, "elev_order");
      order.type = cJSON_GetObjectItem(elev_order, "order_type")->valueint;
      order.floor = cJSON_GetObjectItem(elev_order, "order_floor")->valueint;
    }

    free(tmp);
    cJSON_Delete(root);

    return order;
}
