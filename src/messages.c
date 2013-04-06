#include "messages.h"
#include <string.h>
#include <malloc.h>

#include <cJSON/cJSON.h>

char* messages_order(int floor, int type) {
    char* out;
    cJSON *root, *cli;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "elev_order", cli=cJSON_CreateObject());
    cJSON_AddNumberToObject(cli, "order_type", type);
    cJSON_AddNumberToObject(cli, "order_floor", floor);

    out = cJSON_Print(root);
    cJSON_Delete(root);

    return out;
}
