#include "server.h"
#include "manager.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


Server
server_create(int size_x, int size_y, int managers_count)
{
    Server server;
    server.managers_count = managers_count;
    server.size_x = size_x;
    server.size_y = size_y;

    server.rstripes[0] = 0;
    server.wstripes[0] = 0;
    server.rstripes[managers_count] = 0;
    server.wstripes[managers_count] = 0;

    for (int i = 1; i < managers_count; i++) {
        server.rstripes[i] = shmget(ftok(TOKEN, 2 * i), 2 * size_y, IPC_CREAT | 0666);
        server.wstripes[i] = shmget(ftok(TOKEN, 2 * i + 1), 2 * size_y, IPC_CREAT | 0666);
    }

    server.client_queue = msgget(ftok(TOKEN, 0), IPC_CREAT | 0666);

    for (int i = 0; i < managers_count; i++) {
        int width = size_x / managers_count;
        if (i == managers_count - 1) {
            width += size_x % managers_count;
        }
        server.manager_queues[i] = msgget(ftok(TOKEN, i + 1), IPC_CREAT | 0666);
        server.managers[i] = manager_create(server.manager_queues[i], width, size_y,
                server.rstripes[i], server.wstripes[i],
                server.rstripes[i + 1], server.wstripes[i + 1]);
    }

    for (int i = 0; i < server.managers_count; i++) {
        manager_run(server.managers[i]);
    }

    server.fifo = fopen(FIFO, "w");
    return server;
}

void
server_destroy(Server server)
{
    ManagerCommand command;
    for (int i = 0; i < server.managers_count; i++) {
        command.type = MANAGER_STOP;
        msgsnd(server.manager_queues[i], &command, sizeof(command) - sizeof(long), 0);
    }

    while (wait(NULL) != -1) ;

    for (int i = 0; i < server.managers_count; i++) {
        manager_destroy(server.managers[i]);
    }

    for (int i = 0; i < server.managers_count; i++) {
        msgctl(server.manager_queues[i], IPC_RMID, NULL);
    }

    msgctl(server.client_queue, IPC_RMID, NULL);

    for (int i = 1; i < server.managers_count; i++) {
        shmctl(server.rstripes[i], IPC_RMID, NULL);
        shmctl(server.wstripes[i], IPC_RMID, NULL);
    }

    fclose(server.fifo);
}

void
server_run(Server server)
{
    int iterations = 0;
    int run = 1;
    ServerCommand command;
    while (run) {
        if (iterations) {
            for (int i = 0; i < server.managers_count; i++) {
                command.type = MANAGER_EVALUATE;
                msgsnd(server.manager_queues[i], &command,
                        sizeof(command) - sizeof(long), 0);
            }
            for (int i = 0; i < server.managers_count; i++) {
                msgrcv(server.manager_queues[i], &command,
                        sizeof(command) - sizeof(long), MANAGER_EVALUATED, 0);
            }
            iterations--;
            if (!iterations) {
                printf("Done\n");
            }
        }
        int result = msgrcv(server.client_queue, &command, sizeof(command) - sizeof(long),
                -SERVER_MAX, IPC_NOWAIT);
        if (result == -1) {
            if (!iterations) {
                usleep(TIMEOUT);
            }
            continue;
        }
        int error = 0;
        switch (command.type) {
        case SERVER_ADD:
            printf("SERVER_ADD(%d, %d)\n", command.parameter1, command.parameter2);
            int x = command.parameter1;
            int y = command.parameter2;
            if ((x < 0) || (y < 0) || (x >= server.size_x) || (y >= server.size_y)) {
                error = INVALID_COORDINATES;
            }
            else {
                command.type = MANAGER_ADD;
                int width = server.size_x / server.managers_count;
                int manager_number = command.parameter1 / width;
                if (manager_number == server.managers_count) {
                    manager_number--;
                }
                command.parameter1 -= manager_number * width;
                msgsnd(server.manager_queues[manager_number], &command,
                        sizeof(command) - sizeof(long), 0);
            }
            break;
        case SERVER_CLEAR:
            printf("SERVER_CLEAR\n");
            for (int i = 0; i < server.managers_count; i++) {
                command.type = MANAGER_CLEAR;
                msgsnd(server.manager_queues[i], &command,
                        sizeof(command) - sizeof(long), 0);
            }
            break;
        case SERVER_START:
            printf("SERVER_START(%d)\n", command.parameter1);
            if (iterations) {
                error = ALREADY_STARTED;
            }
            else {
                iterations = command.parameter1;
            }
            break;
        case SERVER_STOP:
            printf("SERVER_STOP\n");
            if (!iterations) {
                error = ALREADY_STOPPED;
            }
            else {
                iterations = 0;
            }
            break;
        case SERVER_SNAPSHOT:
            printf("SERVER_SNAPSHOT\n");
            for (int y = 0; y < server.size_y; y++) {
                char *string = calloc(server.size_x + 1, sizeof(char));
                int pos = 0;
                for (int i = 0; i < server.managers_count; i++) {
                    int width = server.size_x / server.managers_count;
                    if (i == server.managers_count - 1) {
                        width += server.size_x % server.managers_count;
                    }
                    ManagerDatagram datagram;
                    command.type = MANAGER_GET_STRING;
                    command.parameter1 = y;
                    msgsnd(server.manager_queues[i], &command,
                            sizeof(command) - sizeof(long), 0);
                    msgrcv(server.manager_queues[i], &datagram,
                            MAXWIDTH, MANAGER_DATAGRAM, 0);
                    for (int j = pos; j < pos + width; j++) {
                        string[j] = datagram.string[j - pos];
                    }
                    pos += width;
                }
                for (int i = 0; i < server.size_x; i++) {
                    if (string[i]) {
                        string[i] = '*';
                    }
                    else {
                        string[i] = '.';
                    }
                }
                fprintf(server.fifo, "%s\n", string);
                free(string);
            }
            char delimiter = -1;
            fwrite(&delimiter, 1, 1, server.fifo);
            fflush(server.fifo);
            break;
        case SERVER_END:
            printf("SERVER_END\n");
            run = 0;
            break;
        default:
            fprintf(stderr, "Server received invalid message: %ld\n", command.type);
            break;
        }
        command.type = SERVER_ANSWER;
        command.parameter1 = error;
        msgsnd(server.client_queue, &command, sizeof(command) - sizeof(long), 0);
    }
}
