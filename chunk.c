#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

Buffer
buffer_create(int size_x, int size_y, int left_stripe_id, int right_stripe_id)
{
    Buffer buffer;
    buffer.size_x = size_x;
    buffer.size_y = size_y;
    buffer.payload = calloc(size_x, sizeof(char*));
    for (int i = 0; i < size_x; i++) {
        buffer.payload[i] = calloc(size_y, sizeof(char));
    }
    if (left_stripe_id) {
        buffer.left_stripe = shmat(left_stripe_id, NULL, 0);
    }
    else {
        buffer.left_stripe = NULL;
    }
    if (right_stripe_id) {
        buffer.right_stripe = shmat(right_stripe_id, NULL, 0);
    }
    else {
        buffer.right_stripe = NULL;
    }
    return buffer;
}

void
buffer_destroy(Buffer buffer)
{
    for (int i = 0; i < buffer.size_x; i++) {
        free(buffer.payload[i]);
    }
    free(buffer.payload);
    if (buffer.left_stripe) {
        shmdt(buffer.left_stripe);
    }
    if (buffer.right_stripe) {
        shmdt(buffer.right_stripe);
    }
}

Chunk
chunk_create(int size_x, int size_y,
    int left_rstripe, int left_wstripe, int right_rstripe, int right_wstripe)
{
    if ((size_x < 3) || (size_x > 1000)) {
        fprintf(stderr, "chunk_create(%d): invalid parameter\n", size_x);
        exit(1);
    }
    Chunk chunk;
    chunk.size_x = size_x;
    chunk.size_y = size_y;
    chunk.rbuffer = buffer_create(size_x, size_y, left_rstripe, right_rstripe);
    chunk.wbuffer = buffer_create(size_x, size_y, left_wstripe, right_wstripe);
    return chunk;
}

void
chunk_destroy(Chunk chunk)
{
    buffer_destroy(chunk.rbuffer);
    buffer_destroy(chunk.wbuffer);
}

char
chunk_get(Chunk chunk, int x, int y)
{
    if ((x < -1) || (x > chunk.size_x) || (y < 0) || (y > chunk.size_y - 1)) {
        fprintf(stderr, "chunk_get(%d, %d): invalid parameter\n", x, y);
        exit(1);
    }
    if (x == -1) {
        if (chunk.rbuffer.left_stripe) {
            return chunk.rbuffer.left_stripe[y];
        }
        else {
            return 0;
        }
    }
    if (x == 0) {
        if (chunk.rbuffer.left_stripe) {
            return chunk.rbuffer.left_stripe[chunk.size_y + y];
        }
    }
    if (x == chunk.size_x) {
        if (chunk.rbuffer.right_stripe) {
            return chunk.rbuffer.right_stripe[chunk.size_y + y];
        }
        else {
            return 0;
        }
    }
    if (x == chunk.size_x - 1) {
        if (chunk.rbuffer.right_stripe) {
            return chunk.rbuffer.right_stripe[y];
        }
    }
    return chunk.rbuffer.payload[x][y];
}

void
chunk_set(Chunk chunk, int x, int y, char value)
{
    if ((x < 0) || (x > chunk.size_x - 1) || (y < 0) || (y > chunk.size_y - 1)) {
        fprintf(stderr, "chunk_set(%d, %d): invalid parameter\n", x, y);
        exit(1);
    }
    if (x == 0) {
        if (chunk.wbuffer.left_stripe) {
            chunk.wbuffer.left_stripe[chunk.size_y + y] = value;
        }
    }
    if (x == chunk.size_x - 1) {
        if (chunk.wbuffer.right_stripe) {
            chunk.wbuffer.right_stripe[y] = value;
        }
    }
    chunk.wbuffer.payload[x][y] = value;
}

void
chunk_flip(Chunk *chunk)
{
    Buffer tbuffer = chunk->wbuffer;
    chunk->wbuffer = chunk->rbuffer;
    chunk->rbuffer = tbuffer;
}
