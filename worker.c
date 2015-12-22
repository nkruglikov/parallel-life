#include "worker.h"

void
worker_add(Worker worker, int x, int y)
{
    chunk_flip(worker.chunk);
    chunk_set(worker.chunk, x, y, 1);
    chunk_flip(worker.chunk);
}

void
worker_clear(Worker worker)
{
    for (int y = 0; y < worker.chunk->size_y; y++) {
        for (int x = 0; x < worker.chunk->size_x; x++) {
            chunk_set(worker.chunk, x, y, 0);
        }
    }
    chunk_flip(worker.chunk);
}

int
min(int x, int y)
{
    return x < y ? x : y;
}

int
max(int x, int y)
{
    return x > y ? x : y;
}

int
count_neighbors(Chunk *chunk, int x, int y)
{
    int neighbors = 0;
    for (int y1 = max(0, y - 1); y1 <= min(chunk->size_y - 1, y + 1); y1++) {
        for (int x1 = x - 1; x1 <= x + 1; x1++) {
            if ((x1 != x) || (y1 != y)) {
                neighbors += chunk_get(chunk, x1, y1);
            }
        }
    }
    return neighbors;
}

void
worker_evaluate(Worker worker)
{
    for (int y = 0; y < worker.chunk->size_y; y++) {
        for (int x = 0; x < worker.chunk->size_x; x++) {
            int neighbors = count_neighbors(worker.chunk, x, y);
            char current = chunk_get(worker.chunk, x, y);
            if ((neighbors < 2) || (neighbors > 3)) {
                chunk_set(worker.chunk, x, y, 0);
            }
            else if (neighbors == 3) {
                chunk_set(worker.chunk, x, y, 1);
            }
            else {
                chunk_set(worker.chunk, x, y, current);
            }

        }
    }
    chunk_flip(worker.chunk);
}

void
worker_get_string(Worker worker, int y, char *string)
{
    for (int x = 0; x < worker.chunk->size_x; x++) {
        string[x] = chunk_get(worker.chunk, x, y);
    }
}
