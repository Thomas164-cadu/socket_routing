#include "./router_list.h"
#include "./router.h"

router_list *init_router_list()
{
    router_list *list = malloc(sizeof(router_list));
    list->value = -1;
    list->next = NULL;
    return list;
}

router_list *add_int(router_list *list, int value)
{
    pthread_mutex_lock(&r.neighbor_list_lock);
    if (list == NULL)
    {
        router_list *newList = init_router_list();
        newList->value = value;
        pthread_mutex_unlock(&r.neighbor_list_lock);
        return newList;
    }

    if (list->value == -1)
    {
        list->value = value;
        pthread_mutex_unlock(&r.neighbor_list_lock);
        return list;
    }

    else
    {
        router_list *new = malloc(sizeof(router_list));
        new->value = value;
        new->next = list;
        pthread_mutex_unlock(&r.neighbor_list_lock);
        return new;
    }
}

router_list *remove_int(router_list *list, int value)
{
    if (list == NULL)
        return NULL;

    pthread_mutex_lock(&r.neighbor_list_lock);

    if (list->value == value)
    {
        router_list *next = list->next;
        free(list);
        pthread_mutex_unlock(&r.neighbor_list_lock);
        return next;
    }

    router_list *current = list;
    
    while (current->next)
    {
        if (current->next->value == value)
        {
            router_list *next = current->next->next;
            free(current->next);
            current->next = next;
            pthread_mutex_unlock(&r.neighbor_list_lock);
            return list;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&r.neighbor_list_lock);

    return list;
}

int in(router_list *list, int value)
{
    pthread_mutex_lock(&r.neighbor_list_lock);

    for (router_list *iterator = list; iterator != NULL; iterator = iterator->next)
    {
        if (iterator->value == value)
        {
            pthread_mutex_unlock(&r.neighbor_list_lock);
            return true;
        }
    }

    pthread_mutex_unlock(&r.neighbor_list_lock);

    return false;
}