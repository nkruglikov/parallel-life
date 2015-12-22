#ifndef WORKER_H
#define WORKER_H

#include "chunk.h"

typedef struct {
    Chunk *chunk;
} Worker;

void worker_add(Worker worker, int x, int y);
void worker_clear(Worker worker);
void worker_evaluate(Worker worker);
void worker_get_string(Worker worker, int y, char *string);

#endif
