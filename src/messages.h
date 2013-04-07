#ifndef __MESSAGES_H__
#define __MESSAGES_H__

typedef struct {
    int type;
	int floor;
} order_t;

char* messages_create_order(int floor, int type);
order_t messages_parse_order(char *msg);

#endif
