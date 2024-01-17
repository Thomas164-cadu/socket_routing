#include "./router.h"

/**
 * Função responsável por atualizar o vetor distância
 * 
 * @param msg 
 */

void update_distance_vector(message msg)
{
    pthread_mutex_lock(&r.distant_routers_lock);
    pthread_mutex_lock(&r.neighbor_list_lock);

    if (r.distant_routers[msg.sender].id == -1)
    {
        r.distant_routers[msg.sender].id = msg.sender;

        r.distant_routers[msg.sender].last_update = time(NULL);
        r.neighbor_list = add_int(r.neighbor_list, msg.sender);
    }

    int id, cost;
    char *token = strtok(msg.data + 2, "\n");
    bool changed = false;
    while (token != NULL)
    {
        sscanf(token, "%d %d", &id, &cost);

        if (id == r.id)
        {
            token = strtok(NULL, "\n");
            continue;
        }

        if(r.distant_routers[msg.sender].is_neighbor)
            r.distant_routers[msg.sender].distance_vector[id] = cost;

        if (r.distant_routers[id].id == -1)
        {
            r.distant_routers[id].id = id;
            r.distant_routers[id].cost = cost + r.distant_routers[msg.sender].cost;
            r.distant_routers[id].source = msg.sender;

            changed = true;
        }
        else
        {
            if (r.distant_routers[id].source == msg.sender &&
                r.distant_routers[id].cost != cost + r.distant_routers[msg.sender].cost)
            {
                if (cost + r.distant_routers[msg.sender].cost > NETWORK_DIAMETER)
                {
                    r.distant_routers[id].cost = NETWORK_DIAMETER;
                    token = strtok(NULL, "\n");
                    continue;
                }

                r.distant_routers[id].cost = cost + r.distant_routers[msg.sender].cost;
                changed = true;
            }
            else if (r.distant_routers[id].cost > cost + r.distant_routers[msg.sender].cost)
            {
                r.distant_routers[id].cost = cost + r.distant_routers[msg.sender].cost;
                r.distant_routers[id].source = msg.sender;
                changed = true;
            }
        }
        token = strtok(NULL, "\n");
    }
    pthread_mutex_unlock(&r.neighbor_list_lock);
    pthread_mutex_unlock(&r.distant_routers_lock);

    if (changed)
        send_distance_vectors();
}

/**
 * Função que intermedia o controle de roteadores ativos
 * 
 * @param msg 
 */
void handle_control_message(message msg)
{
    int type;
    char log[100];

    sscanf(msg.data, "%d", &type);

    switch (type)
    {

    case DISCONNECT:
        sprintf(log, "Desconectando do roteador: %d\n", msg.origin);
        write_to_log(log);
        disconnect(msg.origin);
        update_distance_vector(msg);
        break;
    case DISTANCE_VECTOR:
        sprintf(log, "Vetor distancia vindo de: %d\n", msg.origin);
        printf("Vetor distancia vindo de: %d\n", msg.origin);
        write_to_log(log);
        update_distance_vector(msg);
        break;
    default:
        die("Tipo de mensagem não identificada");
    }
}

/**
 * Função que intermedia o envio de mensagens de dados
 * 
 * @param msg 
 */
void handle_data_message(message msg)
{
    enqueue(r.messages, msg);
}

/**
 * Função que trata as mensagens do roteador (internas e externas)
 */
void *packet_handler(void *args)
{
    char log[100];
    while (1)
    {
        message msg = dequeue(r.in);
        sprintf(log, "Roteador %d encaminhando mensagem para o destino %d\n", msg.origin, msg.destiny_id);
        write_to_log(log);
        printf("Roteador %d encaminhando mensagem para o destino %d\n", msg.origin, msg.destiny_id);

        if (msg.destiny_id != r.id)
        {
            sprintf(log, "Mensagem encaminhada para: %d", msg.destiny_id);
            write_to_log(log);
            enqueue(r.out, msg);
            continue;
        }

        switch (msg.type)
        {
        case CONTROL:
            handle_control_message(msg);
            break;
        case DATA:
            handle_data_message(msg);
            break;
        default:
            die("Tipo de mensagem não identificada");
        }
    }
}
