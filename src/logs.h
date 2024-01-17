#ifndef LOGS_H
#define LOGS_H
#include <pthread.h>
#define LOG_SIZE 1000

typedef struct
{
    char log[LOG_SIZE];
    int size;
    pthread_mutex_t lock;
} logs;

#endif