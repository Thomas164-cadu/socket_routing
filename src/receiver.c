#include "./router.h"

void *receiver(void *args)
{

    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other), recv_len;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket");

    memset((char *)&si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(r.port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
        die("bind");

    message msg;

    while (1)
    {
        memset(&msg, 0, sizeof(message));

        if ((recv_len = recvfrom(s, &msg, sizeof(message), 0, (struct sockaddr *)&si_other, &slen)) == -1)
            die("recvfrom()");

        pthread_mutex_lock(&r.distant_routers_lock);
        pthread_mutex_lock(&r.neighbor_list_lock);

        r.distant_routers[msg.origin].last_update = time(NULL);
        r.distant_routers[msg.sender].last_update = time(NULL);

        pthread_mutex_unlock(&r.neighbor_list_lock);
        pthread_mutex_unlock(&r.distant_routers_lock);

        char log[100];

        sprintf(log, "Mensagem recebida de: %d", msg.origin);
        write_to_log(log);
        enqueue(r.in, msg);
    }
}