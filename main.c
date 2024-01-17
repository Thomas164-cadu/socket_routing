#include "src/router.h"
#include <sys/types.h>
#include <sys/stat.h>
#define NO_THREADS 6

router r;

/**
 * O arquivo main é responsável por inicializar os roteadores e chamar as funções a partir dos arquivos de implementação
 */

int main(int argc, char const *argv[])
{
    atexit(append_logs);
    if (argc != 2)
    {
        puts("Roteador: ./router <id>");
        return 1;
    }

    struct stat st = {0};
    if (stat("logs", &st) == -1)
        mkdir("logs", 0700);

    char filename[20];

    sprintf(filename, "logs/router%s.log", argv[1]);

    FILE *f = fopen(filename, "a");
    
    fclose(f);

    init_router(atoi(argv[1]));

    pthread_t threads[NO_THREADS];

    void *(*func[NO_THREADS])(void *) = {
        terminal,
        sender,
        receiver,
        packet_handler,
        check_to_resend_distance_vector,
        check_neighbors};

    void *args[NO_THREADS];

    for (int i = 0; i < NO_THREADS; i++)
        args[i] = NULL;

    for (int i = 0; i < NO_THREADS; i++)
        pthread_create(&threads[i], NULL, func[i], args[i]);

    for (int i = 0; i < NO_THREADS; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
