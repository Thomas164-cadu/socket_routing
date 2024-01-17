#include "./queue.h"

queue *init_queue()
{
    queue *q = malloc(sizeof(queue));

    memset(q->buffer, -1, sizeof(message) * BUFFER_SIZE);

    q->head = q->buffer;
    q->tail = q->buffer;
    q->semaphore = calloc(1, sizeof(sem_t));

    sem_init(q->semaphore, 0, 0);

    pthread_mutex_init(&q->mutex, NULL);
    return q;
}

queue_status enqueue(queue *q, message msg)
{
    pthread_mutex_lock(&q->mutex);

    int sem_value;

    sem_getvalue(q->semaphore, &sem_value);

    if (sem_value >= BUFFER_SIZE || q->tail->destiny_id != -1)
    {
        pthread_mutex_unlock(&q->mutex);
        return QUEUE_FULL;
    }

    *q->tail = msg;
    q->tail++;

    //printf("Roteador %d encaminhando mensagem para o destino %d\n", msg.origin, msg.destiny_id);

    if (q->tail == q->buffer + BUFFER_SIZE)
        q->tail = q->buffer;

    sem_post(q->semaphore);

    pthread_mutex_unlock(&q->mutex);

    return QUEUE_OK;
}

message dequeue(queue *q)
{
    sem_wait(q->semaphore);

    pthread_mutex_lock(&q->mutex);

    if (q->head->destiny_id == -1)
    {
        pthread_mutex_unlock(&q->mutex);
        return (message){0};
    }

    message msg = *q->head;
    memset(q->head, -1, sizeof(message));

    q->head++;

    if (q->head == q->buffer + BUFFER_SIZE)
        q->head = q->buffer;

    pthread_mutex_unlock(&q->mutex);
    
    return msg;
}

void print_queue(queue *q)
{
    pthread_mutex_lock(&q->mutex);

    for (message *i = q->head; i != q->tail; i++)
    {
        if (i == q->buffer + BUFFER_SIZE)
            i = q->buffer;
        printf("Mensagem de %d: %s", i->origin, i->data);
    }

    pthread_mutex_unlock(&q->mutex);
}