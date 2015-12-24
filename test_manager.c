#include "manager.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

void
manager_test_1(void)
{
    printf("MANAGER_TEST_1\n");
    int stripe11 = shmget(ftok("life.token", 11), 10 * 2, IPC_CREAT | 0666);
    int stripe12 = shmget(ftok("life.token", 12), 10 * 2, IPC_CREAT | 0666);
    int stripe21 = shmget(ftok("life.token", 21), 10 * 2, IPC_CREAT | 0666);
    int stripe22 = shmget(ftok("life.token", 22), 10 * 2, IPC_CREAT | 0666);

    int msg1 = msgget(ftok("life.token", 11), IPC_CREAT | 0666);
    int msg2 = msgget(ftok("life.token", 12), IPC_CREAT | 0666);
    int msg3 = msgget(ftok("life.token", 13), IPC_CREAT | 0666);

    Manager manager1 = manager_create(msg1, 10, 10, 0, 0, stripe11, stripe12);
    Manager manager2 = manager_create(msg2, 10, 10,
            stripe11, stripe12, stripe21, stripe22);
    Manager manager3 = manager_create(msg3, 10, 10, stripe21, stripe22, 0, 0);

    manager_run(manager1);
    manager_run(manager2);
    manager_run(manager3);

    char points[9][2] = {{5, 1}, {8, 1}, {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {7, 4}, {8, 3}};

    ManagerCommand command;
    ManagerDatagram datagram;
    for (int point = 0; point < 9; point++) {
        command.type = MANAGER_ADD;
        command.parameter1 = points[point][0];
        command.parameter2 = points[point][1];
        msgsnd(msg3, &command, sizeof(command) - sizeof(long), 0);
    }

    for (int step = 0; step < 40; step++) {
        printf("Step %d\n", step + 1);
        for (int y = 0; y < 10; y++) {
            command.type = MANAGER_GET_STRING;
            command.parameter1 = y;
            msgsnd(msg1, &command, sizeof(command) - sizeof(long), 0);
            msgrcv(msg1, &datagram, sizeof(datagram) - sizeof(long), MANAGER_DATAGRAM, 0);
            for (int x = 0; x < 10; x++) {
                if (datagram.string[x]) {
                    printf("*");
                }
                else {
                    printf(".");
                }
            }
            printf("\n");
        }
        command.type = MANAGER_EVALUATE;
        msgsnd(msg1, &command, sizeof(command) - sizeof(long), 0);
        command.type = MANAGER_EVALUATE;
        msgsnd(msg2, &command, sizeof(command) - sizeof(long), 0);
        command.type = MANAGER_EVALUATE;
        msgsnd(msg3, &command, sizeof(command) - sizeof(long), 0);

        printf("Commands to managers are sent.\n");
        msgrcv(msg1, &command, sizeof(command) - sizeof(long), MANAGER_EVALUATED, 0);
        msgrcv(msg2, &command, sizeof(command) - sizeof(long), MANAGER_EVALUATED, 0);
        msgrcv(msg3, &command, sizeof(command) - sizeof(long), MANAGER_EVALUATED, 0);
    }

    command.type = MANAGER_STOP;
    msgsnd(msg1, &command, sizeof(command) - sizeof(long), 0);
    command.type = MANAGER_STOP;
    msgsnd(msg2, &command, sizeof(command) - sizeof(long), 0);
    command.type = MANAGER_STOP;
    msgsnd(msg3, &command, sizeof(command) - sizeof(long), 0);

    manager_destroy(manager1);
    manager_destroy(manager2);
    manager_destroy(manager3);
}

int
main(void)
{
    manager_test_1();
    return 0;
}
