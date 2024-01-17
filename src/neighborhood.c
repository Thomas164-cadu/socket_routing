#include "./router.h"

/**
 * Funçao que verifica os vizinhos do roteador
 * 
 * @param arg argumento passado para a thread
 */

void *check_neighbors(void *arg)
{
    char log[100];
    while (1)
    {
        usleep(CHECK_NEIGHBORS_DELAY);

    keep_going:

        pthread_mutex_lock(&r.distant_routers_lock);
        pthread_mutex_lock(&r.neighbor_list_lock);

        for (router_list *iterator = r.neighbor_list; iterator; iterator = iterator->next)
        {
            if (difftime(time(NULL), r.distant_routers[iterator->value].last_update) > TIME_OUT && r.distant_routers[iterator->value].id != -1)
            {
                sprintf(log, "Vizinho %d desconectado\n", iterator->value);

                write_to_log(log);

                pthread_mutex_unlock(&r.neighbor_list_lock);
                pthread_mutex_unlock(&r.distant_routers_lock);

                disconnect(iterator->value);
                
                goto keep_going;
            }
                }

        pthread_mutex_unlock(&r.neighbor_list_lock);
        pthread_mutex_unlock(&r.distant_routers_lock);
    }
}