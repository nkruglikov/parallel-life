#include "server.h"

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

Server server;

void
clean(int sig) {
    server_destroy(server);
    printf("Killed by SIGINT\n");
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Invalid number of command-line arguments\n");
        exit(1);
    }

    signal(SIGINT, clean);

    int size_y = atoi(argv[1]);
    int size_x = atoi(argv[2]);
    int client_num = atoi(argv[3]);
    printf("Creating server...\n");
    server = server_create(size_x, size_y, client_num);
    printf("Server created\n");
    server_run(server);
    server_destroy(server);
    printf("Server destroyed\n");
    return 0;
}
