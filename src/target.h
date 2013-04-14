#ifndef __TARGET_H__
#define __TARGET_H__

#include "orderlist.h"

typedef enum {
    TYPE_COMMAND,
    TYPE_CALL_UP,
    TYPE_CALL_DOWN,
    N_TYPES,
} target_type_t;

int target_update (double floor);
void target_clear_completed_order (void);

#endif
