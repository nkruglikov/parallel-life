#ifndef CHUNK_H
#define CHUNK_H

typedef struct {
    char** payload;
    char* left_stripe;
    char* right_stripe;
    int size_x;
    int size_y;
} Buffer;

typedef struct {
    Buffer rbuffer;
    Buffer wbuffer;
    int size_x;
    int size_y;
} Chunk;

Buffer buffer_create(int size_x, int size_y, int left_stripe_id, int right_stripe_id);
void buffer_destroy(Buffer buffer);

Chunk chunk_create(int size_x, int size_y,
    int left_rstripe, int left_wstripe, int right_rstripe, int right_wstripe);
void chunk_destroy(Chunk chunk);

char chunk_get(Chunk chunk, int x, int y);
void chunk_set(Chunk chunk, int x, int y, char value);
void chunk_flip(Chunk *chunk);

#endif
