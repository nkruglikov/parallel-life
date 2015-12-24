#include "manager.h"
#include "chunk.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
 

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

Manager
manager_create(int msg_id, int size_x, int size_y,
    int left_rstripe, int left_wstripe, int right_rstripe, int right_wstripe)
{
    Manager manager;
    Chunk *chunk = chunk_create(size_x, size_y,
            left_rstripe, left_wstripe, right_rstripe, right_wstripe);

    manager.worker.chunk = chunk;
    manager.msg_id = msg_id;
    return manager;
}

void
manager_destroy(Manager manager)
{
    chunk_destroy(manager.worker.chunk);
}

pid_t
manager_run(Manager manager)
{
    pid_t pid;
    if (!(pid = fork())) {
        int run = 1;
        while (run) {
            ManagerCommand command;
            ManagerDatagram datagram;
            msgrcv(manager.msg_id, &command, sizeof(command) - sizeof(long),
                    -MANAGER_MAX, 0);
            switch (command.type) {
            case MANAGER_EVALUATE:
                worker_evaluate(manager.worker);
                command.type = MANAGER_EVALUATED;
                msgsnd(manager.msg_id, &command, sizeof(command) - sizeof(long),
                        IPC_NOWAIT);
                break;
            case MANAGER_ADD:
                worker_add(manager.worker, command.parameter1, command.parameter2);
                break;
            case MANAGER_CLEAR:
                worker_clear(manager.worker);
                break;
            case MANAGER_STOP:
                run = 0;
                break;
            case MANAGER_GET_STRING:
                worker_get_string(manager.worker, command.parameter1, datagram.string);
                datagram.type = MANAGER_DATAGRAM;
                msgsnd(manager.msg_id, &datagram, MAXWIDTH, 0);
                break;
            default:
                fprintf(stderr, "Manager received invalid message: %ld\n", command.type);
                break;
            }
        }
        exit(0);
    }
    return pid;
}
