#ifndef SERVER_H
#define SERVER_H

#include "manager.h"
#include <stdio.h>

#define TOKEN "life.token"
#define FIFO "life.fifo"

enum
{
    MAX_MANAGERS = 100,
    TIMEOUT = 300
};

typedef struct
{
    int managers_count;
    int client_queue;
    int size_x;
    int size_y;
    Manager managers[MAX_MANAGERS];
    int manager_queues[MAX_MANAGERS];
    int rstripes[MAX_MANAGERS + 1];
    int wstripes[MAX_MANAGERS + 1];
    FILE *fifo;
} Server;

enum
{
    SERVER_ADD = 1,
    SERVER_CLEAR,
    SERVER_START,
    SERVER_STOP,
    SERVER_SNAPSHOT,
    SERVER_END,
    SERVER_MAX,
    SERVER_ANSWER
};

enum
{
    ALREADY_STARTED = 1,
    ALREADY_STOPPED,
    INVALID_COORDINATES
};

typedef struct
{
    long type;
    int parameter1;
    int parameter2;
} ServerCommand;

Server server_create(int size_x, int size_y, int managers_count);
void server_destroy(Server server);
void server_run(Server server);

#endif
