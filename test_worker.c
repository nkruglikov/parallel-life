#include "chunk.h"
#include "worker.h"

#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

void
test_worker_1(void)
{
    printf("TEST_WORKER_1\n");
    Chunk *chunk = chunk_create(10, 10, 0, 0, 0, 0);
    Worker worker = {chunk};

    char points[5][2] = {{2, 1}, {3, 2}, {1, 3}, {2, 3}, {3, 3}};
    for (int point = 0; point < 5; point++) {
        worker_add(worker, points[point][0], points[point][1]);
    }
    for (int step = 0; step < 10; step++) {
        worker_evaluate(worker);
        printf("Step %d\n", step + 1);
        char string[10];
        for (int y = 0; y < 10; y++) {
            worker_get_string(worker, y, string);
            for (int x = 0; x < 10; x++) {
                if (string[x]) {
                    printf("*");
                }
                else {
                    printf(".");
                }
            }
            printf("\n");
        }
    }

    chunk_destroy(chunk);
}

void
test_worker_2(void)
{
    printf("TEST_WORKER_2\n");
    int stripe11 = shmget(ftok("life.token", 11), 10 * 2, IPC_CREAT | 0666);
    int stripe12 = shmget(ftok("life.token", 12), 10 * 2, IPC_CREAT | 0666);
    int stripe21 = shmget(ftok("life.token", 21), 10 * 2, IPC_CREAT | 0666);
    int stripe22 = shmget(ftok("life.token", 22), 10 * 2, IPC_CREAT | 0666);

    Chunk *chunk1 = chunk_create(10, 10, 0, 0, stripe11, stripe12);
    Chunk *chunk2 = chunk_create(10, 10, stripe11, stripe12, stripe21, stripe22);
    Chunk *chunk3 = chunk_create(10, 10, stripe21, stripe22, 0, 0);

    Worker worker1 = {chunk1};
    Worker worker2 = {chunk2};
    Worker worker3 = {chunk3};

    char points[9][2] = {{5, 1}, {8, 1}, {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {7, 4}, {8, 3}};
    for (int point = 0; point < 9; point++) {
        worker_add(worker3, points[point][0], points[point][1]);
    }

    for (int step = 0; step < 40; step++) {
        printf("Step %d\n", step + 1);
        char string[30];
        for (int y = 0; y < 10; y++) {
            worker_get_string(worker1, y, string);
            worker_get_string(worker2, y, string + 10);
            worker_get_string(worker3, y, string + 20);
            for (int x = 0; x < 30; x++) {
                if (string[x]) {
                    printf("*");
                }
                else {
                    printf(".");
                }
            }
            printf("\n");
        }
        worker_evaluate(worker1);
        worker_evaluate(worker2);
        worker_evaluate(worker3);
    }

    chunk_destroy(chunk1);
    chunk_destroy(chunk2);
    chunk_destroy(chunk3);

    shmctl(stripe11, IPC_RMID, NULL);
    shmctl(stripe12, IPC_RMID, NULL);
    shmctl(stripe21, IPC_RMID, NULL);
    shmctl(stripe22, IPC_RMID, NULL);
}

int
main(void)
{
    test_worker_1();
    test_worker_2();
    return 0;
}
