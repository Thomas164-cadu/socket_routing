#include "./router.h"

#define NO_OPTIONS 6

/**
 * Função que lista os vizinhos do roteador
 */
void show_neighbors()
{
    pthread_mutex_lock(&r.distant_routers_lock);
    pthread_mutex_lock(&r.neighbor_list_lock);

    for (router_list *counter = r.neighbor_list; counter; counter = counter->next)
    {
        printf("%d %s:%d\n",
               counter->value,
               r.distant_routers[counter->value].network_info.ip,
               r.distant_routers[counter->value].network_info.port);
    }

    pthread_mutex_unlock(&r.neighbor_list_lock);
    pthread_mutex_unlock(&r.distant_routers_lock);
}

/**
 * Função que lista todos os roteadores disponiveis para envio de mensagem
 */
void list_possible_routers(){
    pthread_mutex_lock(&r.distant_routers_lock);

    for (int i = 0; i < NETWORK_SIZE; i++)
    {
        if (r.distant_routers[i].id != -1)
            if (r.distant_routers[i].source == -1)
                printf(
                    "Roteador %d com custo %d (vizinho)\n",
                    r.distant_routers[i].id,
                    r.distant_routers[i].cost);
            else
                printf(
                    "Roteador %d com custo %d (nao e vizinho), passando por %d\n",
                    r.distant_routers[i].id,
                    r.distant_routers[i].cost,
                    r.distant_routers[i].source);
    }

    pthread_mutex_unlock(&r.distant_routers_lock);

}

/**
 * Função que lista tabela de roteamento
 */
void list_routing_table()
{
    pthread_mutex_lock(&r.distant_routers_lock);
    pthread_mutex_lock(&r.neighbor_list_lock);
    printf("%d %d", -1, r.id);
    for (int i = 0; i < NETWORK_SIZE; i++)
        printf("\t%d", i);
    putchar('\n');
    for (router_list *iterator = r.neighbor_list; iterator != NULL; iterator = iterator->next)
    {
        printf("%d %d", iterator->value, r.distant_routers[iterator->value].cost);
        for (int i = 0; i < NETWORK_SIZE; i++)
            printf("\t%d", r.distant_routers[iterator->value].distance_vector[i]);
        putchar('\n');
    }
    pthread_mutex_unlock(&r.neighbor_list_lock);
    pthread_mutex_unlock(&r.distant_routers_lock);
}

/**
 * Função que lista as mensagens recebidas pelo roteador
 */
void show_messages()
{
    print_queue(r.messages);
}

/**
 * Função responsável por enviar uma mensagem
 */
void send_message()
{
    puts("Para quem deseja enviar a mensagem?\n");
    list_possible_routers();

    char input[2];
    fgets(input, 2, stdin);
    clean_stdin();

    message msg = {
        .type = DATA,
        .destiny_id = *input - '0',
        .origin = r.id,
        .sequence = 0,
    };

    puts("Digite sua mensagem:\n");

    fgets(msg.data, MESSAGE_SIZE, stdin);
    clean_stdin();

    puts("Mensagem enviada!\n");
    write_to_log("Mensagem enviada!\n");
    enqueue(r.out, msg);
}


void exit_router()
{
    exit(0);
}

void menu()
{
    printf("\n");
    char *options[NO_OPTIONS] = {
        "Listar vizinhos",
        "Listar roteadores conhecidos",
        "Listar tabela de roteamento",
        "Mostrar mensagens",
        "Enviar mensagem",
        "Sair"
    };

    void (*functions[NO_OPTIONS])(void) = {show_neighbors, list_possible_routers, list_routing_table, show_messages, send_message, exit_router};

    for (int i = 0; i < NO_OPTIONS; i++)
        printf("%d - %s\n", i, options[i]);

    printf(": ");
    char input[2];
    fgets(input, 2, stdin);
    clean_stdin();

    if (*input < '0' || *input - '0' >= NO_OPTIONS)
        puts("Valor inserido é inválido\n");
    else
        functions[*input - '0']();
}

void *terminal(void *args)
{

    while (1)
        menu();
}