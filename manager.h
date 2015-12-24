#ifndef MANAGER_H
#define MANAGER_H

#include "worker.h"
#include <sys/types.h>

enum
{
    MANAGER_EVALUATE = 1,
    MANAGER_GET_STRING,
    MANAGER_ADD,
    MANAGER_CLEAR,
    MANAGER_STOP,
    MANAGER_MAX,
    MANAGER_DATAGRAM,
    MANAGER_EVALUATED
};

typedef struct
{
    Worker worker;
    int msg_id;
} Manager;

typedef struct
{
    long type;
    int parameter1;
    int parameter2;
} ManagerCommand;

typedef struct
{
    long type;
    char string[MAXWIDTH];
} ManagerDatagram;

Manager manager_create(int msg_id, int size_x, int size_y,
    int left_rstripe, int left_wstripe, int right_rstripe, int right_wstripe);
void manager_destroy(Manager manager);
pid_t manager_run(Manager manager);

#endif
