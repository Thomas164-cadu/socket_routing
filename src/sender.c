#include "./router.h"

void send_distance_vectors()
{
    message msg = {
        .type = CONTROL,
        .origin = r.id,
        .destiny_id = -1,
        .sequence = 0,
    };
    char buffer[MESSAGE_SIZE];
    msg.data[0] = '\0';
    sprintf(msg.data, "%d\n", DISTANCE_VECTOR);

    pthread_mutex_lock(&r.distant_routers_lock);
    pthread_mutex_lock(&r.neighbor_list_lock);

    for (int i = 0; i < NETWORK_SIZE; i++)
    {

        sprintf(buffer, "%d %d\n", r.distant_routers[i].id, r.distant_routers[i].cost);
        if (strlen(msg.data) + strlen(buffer) > MESSAGE_SIZE)
            break;

        if (r.distant_routers[i].id != -1)
            strcat(msg.data, buffer);
    }

    for (router_list *iterator = r.neighbor_list; iterator; iterator = iterator->next)
    {
        msg.destiny_id = iterator->value;
        enqueue(r.out, msg);
    }

    pthread_mutex_unlock(&r.neighbor_list_lock);
    pthread_mutex_unlock(&r.distant_routers_lock);
}

void *check_to_resend_distance_vector(void *args)
{

    while (1)
    {
        usleep(DISTANCE_VECTOR_DELAY);
        send_distance_vectors();
    }
}

/**
 * Sender que envia mensagens para os roteadores vizinhos
 * Também gerencia a fila de mensagens de saída
 */
void *sender(void *args)
{

    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;

    while (1)
    {

        message msg = dequeue(r.out);
        if (msg.destiny_id >= NETWORK_SIZE || msg.destiny_id < 0)
            continue;

        pthread_mutex_lock(&r.distant_routers_lock);
        
        if (r.distant_routers[msg.destiny_id].id == -1 || msg.sequence == NETWORK_SIZE)
        {
            pthread_mutex_unlock(&r.distant_routers_lock);
            continue;
        }

        msg.sequence++;
        msg.sender = r.id;

        int port;
        char *ip;

        int dest = msg.destiny_id;
        while (r.distant_routers[dest].source != -1)
            dest = r.distant_routers[dest].source;

        port = r.distant_routers[dest].network_info.port;
        ip = r.distant_routers[dest].network_info.ip;

        pthread_mutex_unlock(&r.distant_routers_lock);

        si_other.sin_port = htons(port);

        if (inet_aton(ip, &si_other.sin_addr) == 0)
        {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
        }

        if (sendto(s, &msg, sizeof(message), 0, (struct sockaddr *)&si_other, slen) == -1)
        {
            die("sendto()");
        }

        char buffer[MESSAGE_SIZE];

        sprintf(buffer, "Pacote enviado para %s:%d\n", ip, port);

        write_to_log(buffer);
    }
}