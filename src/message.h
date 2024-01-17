#ifndef MESSAGE_H
#define MESSAGE_H

#define MESSAGE_SIZE 100

enum message_type
{
    INVALID,
    CONTROL,
    DATA
};

enum control_type
{
    DISCONNECT,
    DISTANCE_VECTOR
};

typedef struct message
{
    enum message_type type;
    int origin;
    int sender;
    int sequence;
    int destiny_id;

    char data[MESSAGE_SIZE];
} message;

#endif