#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "chunk.h"

void
test_chunk_1(void)
{
    printf("TEST_CHUNK_1\n");
    Chunk chunk1 = chunk_create(10, 10, 0, 0, 0, 0);
    Chunk chunk2 = chunk_create(10, 10, 0, 0, 0, 0);

    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            chunk_set(chunk1, x, y, x * y);
        }
    }
    chunk_flip(&chunk1);

    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            chunk_set(chunk2, x, y, -x * y);
        }
    }
    chunk_flip(&chunk2);

    printf("chunk1:\n");
    for (int y = 0; y < 10; y++) {
        for (int x = -1; x <= 10; x++) {
            printf("%3d ", chunk_get(chunk1, x, y));
        }
        printf("\n");
    }

    printf("chunk2:\n");
    for (int y = 0; y < 10; y++) {
        for (int x = -1; x <= 10; x++) {
            printf("%3d ", chunk_get(chunk2, x, y));
        }
        printf("\n");
    }

    chunk_destroy(chunk1);
    chunk_destroy(chunk2);
}

void
test_chunk_2(void)
{
    printf("TEST_CHUNK_2\n");
    int stripe11 = shmget(ftok("life.token", 11), 10 * 2, IPC_CREAT | 0666);
    int stripe12 = shmget(ftok("life.token", 12), 10 * 2, IPC_CREAT | 0666);
    int stripe21 = shmget(ftok("life.token", 21), 10 * 2, IPC_CREAT | 0666);
    int stripe22 = shmget(ftok("life.token", 22), 10 * 2, IPC_CREAT | 0666);

    Chunk chunk1 = chunk_create(10, 10, 0, 0, stripe11, stripe12);
    Chunk chunk2 = chunk_create(10, 10, stripe11, stripe12, stripe21, stripe22);
    Chunk chunk3 = chunk_create(10, 10, stripe21, stripe22, 0, 0);

    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            chunk_set(chunk2, x, y, x + y);
        }
    }
    chunk_flip(&chunk1);
    chunk_flip(&chunk2);
    chunk_flip(&chunk3);

    printf("chunk1:\n");
    for (int y = 0; y < 10; y++) {
        for (int x = -1; x <= 10; x++) {
            printf("%3d ", chunk_get(chunk1, x, y));
        }
        printf("\n");
    }

    printf("chunk2:\n");
    for (int y = 0; y < 10; y++) {
        for (int x = -1; x <= 10; x++) {
            printf("%3d ", chunk_get(chunk2, x, y));
        }
        printf("\n");
    }

    printf("chunk3:\n");
    for (int y = 0; y < 10; y++) {
        for (int x = -1; x <= 10; x++) {
            printf("%3d ", chunk_get(chunk3, x, y));
        }
        printf("\n");
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
    test_chunk_1();
    test_chunk_2();
    return 0;
}
