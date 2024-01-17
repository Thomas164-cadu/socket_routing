#include "./router.h"

/**
 * Grupo de funções auxiliares
 */
void clean_stdin(void)
{
    int c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

void die(char *s)
{
    perror(s);
    exit(1);
}

FILE *open_file(char *folder, char *filename, char *mode)
{

    int path_len = strlen(folder) + strlen(filename) + 1;

    char path[path_len];
    strcpy(path, folder);
    strcat(path, filename);

    FILE *f = fopen(path, mode);
    if (f == NULL)
    {
        char *template = "Error opening %s file!\n";
        int len = strlen(template) + strlen(path) + 1;
        char msg[len];
        snprintf(msg, len, template, path);
        die(msg);
    }
    return f;
}

/**
 * Função que escreve no arquivo de log
 * 
 * @param s recebe uma string
 */
void write_to_log(char *s)
{
    pthread_mutex_lock(&r.log.lock);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time[50];
    sprintf(time, "%04d/%02d/%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    if (r.log.size + strlen(s) + strlen(time) + 1 > LOG_SIZE)
    {
        append_logs();
        strcpy(r.log.log, "");
        r.log.size = 0;
    }

    strcat(r.log.log, time);
    strcat(r.log.log, s);
    strcat(r.log.log, "\n");

    r.log.size += strlen(s) + strlen(time) + strlen("\n");

    pthread_mutex_unlock(&r.log.lock);
}

void append_logs()
{
    if (r.id == -1 || r.log.size == 0)
        return;

    char filename[50];
    sprintf(filename, "router%d.log", r.id);
    FILE *f = open_file("logs/", filename, "a");

    if (f != NULL)
        fwrite(r.log.log, r.log.size, 1, f);
    fclose(f);
}

void load_logs()
{

    pthread_mutex_lock(&r.log.lock);

    char filename[50] = "router%d.log";

    sprintf(filename, filename, r.id);

    FILE *f = open_file("logs/", filename, "r");

    if (f != NULL)
    {
        fseek(f, -LOG_SIZE, SEEK_END);
        fread(r.log.log, LOG_SIZE, 1, f);
        r.log.size = strlen(r.log.log);
    }

    fclose(f);

    pthread_mutex_unlock(&r.log.lock);
}

void disconnect(int id)
{

    pthread_mutex_lock(&r.distant_routers_lock);
    pthread_mutex_lock(&r.neighbor_list_lock);

    for (int i = 0; i < NETWORK_SIZE; i++)
    {
        if (r.distant_routers[i].source == id)
        {
            r.distant_routers[i].id = -1;
            r.distant_routers[i].cost = -1;
            int lower_cost = -1;
            int lower_cost_id = -1;
            for (router_list *iterator = r.neighbor_list; iterator != NULL; iterator = iterator->next)
            {
                if (iterator->value == id)
                {
                    continue;
                }
                if (lower_cost == -1 || r.distant_routers[iterator->value].cost < lower_cost)
                {
                    lower_cost = r.distant_routers[iterator->value].cost;
                    lower_cost_id = iterator->value;
                }
            }

            r.distant_routers[i].cost = lower_cost;
            r.distant_routers[i].source = lower_cost_id;
        }
    }
    memset(r.distant_routers[id].distance_vector, -1, sizeof(int) * NETWORK_SIZE);

    r.distant_routers[id].id = -1;
    r.distant_routers[id].source = -1;

    pthread_mutex_unlock(&r.neighbor_list_lock);
    pthread_mutex_unlock(&r.distant_routers_lock);
}

void init_router(int id)
{
    r.in = init_queue();
    r.out = init_queue();
    r.messages = init_queue();

    r.id = -1;
    r.neighbor_list = init_router_list();

    for (int i = 0; i < NETWORK_SIZE; i++)
    {
        r.distant_routers[i] = (distant_router){
            .id = -1,
            .cost = -1,
            .source = -1,
            .is_neighbor = false,
            .distance_vector = NULL,
        };
    }

    FILE *f;
    distant_router o;

    f = open_file("config/", "enlaces.config", "r");

    struct enlace
    {
        int source;
        int destiny;
        int cost;
    } enlace;

    while (
        fscanf(f, "%d %d %d", &enlace.source, &enlace.destiny, &enlace.cost) != EOF)
    {
        int addTo = -1;

        if (enlace.source == id)
            addTo = enlace.destiny;
        else if (enlace.destiny == id)
            addTo = enlace.source;
        else
            continue;

        r.neighbor_list = add_int(r.neighbor_list, addTo);
        r.distant_routers[addTo].id = addTo;
        r.distant_routers[addTo].cost = enlace.cost;
        r.distant_routers[addTo].is_neighbor = true;
        r.distant_routers[addTo].distance_vector = malloc(sizeof(int) * NETWORK_SIZE);

        memset(r.distant_routers[addTo].distance_vector, -1, sizeof(int) * NETWORK_SIZE);
    }

    fclose(f);

    f = open_file("config/", "roteador.config", "r");

    while (
        fscanf(f, "%d %d %s", &o.id, &o.network_info.port, o.network_info.ip) != EOF)
    {
        if (o.id == id)
        {
            r.id = o.id;
            r.port = o.network_info.port;
            strcpy(r.ip, o.network_info.ip);
        }

        if (in(r.neighbor_list, o.id))
        {
            r.distant_routers[o.id].network_info.port = o.network_info.port;
            strcpy(r.distant_routers[o.id].network_info.ip, o.network_info.ip);
        }
    }

    fclose(f);
    
    if (r.id == -1)
        die("Roteador não encontrado");

    pthread_mutex_init(&r.neighbor_list_lock, NULL);
    pthread_mutex_init(&r.distant_routers_lock, NULL);
    pthread_mutex_init(&r.log.lock, NULL);

    load_logs();

    printf("Roteador %d :)\n", r.id);
    return;
}
