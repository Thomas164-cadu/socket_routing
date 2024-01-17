#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "message.h"
#include <semaphore.h>
#include <pthread.h>

#define BUFFER_SIZE 12

typedef struct queue
{
    message *head;
    message *tail;
    message buffer[BUFFER_SIZE];
    sem_t *semaphore;
    pthread_mutex_t mutex;

} queue;

typedef enum queue_status
{
    QUEUE_OK,
    QUEUE_FULL,
    QUEUE_EMPTY
} queue_status;

queue *init_queue();
queue_status enqueue(queue *q, message msg);
message dequeue(queue *q);
void print_queue(queue *q);

#endif