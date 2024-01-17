#ifndef ROUTER_LIST_H
#define ROUTER_LIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct node
{
    int value;
    struct node *next;

} router_list;

router_list *init_router_list();
router_list *add_int(router_list *list, int value);
router_list *remove_int(router_list *list, int value);

int in(router_list *list, int value);

#endif