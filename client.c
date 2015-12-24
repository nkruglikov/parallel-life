#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

#define MAX_COMMAND 100

char *ERROR[3] = {
    "Field is being evaluated now",
    "Evaluation had stopped already",
    "Coordinates are out of field bounds"
};

typedef struct
{
    int server_queue;
    FILE *fifo;
} Client;

Client
client_create(void)
{
    Client client;
    client.server_queue = msgget(ftok(TOKEN, 0), 0666);
    if (client.server_queue == -1) {
        fprintf(stderr, "Can't connect to the server\n");
        exit(1);
    }
    client.fifo = fopen(FIFO, "r+");
    return client;
}

void
client_destroy(Client client)
{
    fclose(client.fifo);
}

int
parse(char *string, int *command_code, int *arguments)
{
    char command[MAX_COMMAND];
    char *commands[6] = {"add", "clear", "start", "stop", "snapshot", "quit"};
    int arg_counts[6] = {2, 0, 1, 0, 0, 0};
    int command_codes[6] = {SERVER_ADD, SERVER_CLEAR, SERVER_START, SERVER_STOP,
        SERVER_SNAPSHOT, SERVER_END};
    int arg_count = sscanf(string, "%s %d %d", command, &arguments[0], &arguments[1]);

    int answer = -1;
    for (int i = 0; i < 6; i++) {
        if (!strcmp(command, commands[i]) && (arg_count - 1 >= arg_counts[i])) {
            answer = i;
            *command_code = command_codes[i];
            break;
        }
    }
    return answer == -1 ? -1 : 0;
}


void
client_run(Client client)
{
    int run = 1;
    while (run) {
        char string[MAX_COMMAND];
        if (!fgets(string, MAX_COMMAND, stdin)) {
            run = 0;
            printf("Client got EOF. Stop.\n");
        }
        else {
            int command_code;
            int arguments[2];
            if (parse(string, &command_code, arguments) == -1) {
                printf("Invalid command\n");
                continue;
            }
            ServerCommand command;
            command.type = command_code;
            command.parameter1 = arguments[0];
            command.parameter2 = arguments[1];
            msgsnd(client.server_queue, &command, sizeof(command) - sizeof(long), 0);
            msgrcv(client.server_queue, &command, sizeof(command) - sizeof(long),
                    SERVER_ANSWER, 0);
            if (command.parameter1 == 0) {
                printf("OK\n");
            }
            else {
                printf("ERROR %s\n", ERROR[command.parameter1 - 1]);
            }
            if (command_code == SERVER_END) {
                run = 0;
            }
            if (command_code == SERVER_SNAPSHOT) {
                char byte;
                printf("Field:\n");
                while (fread(&byte, 1, 1, client.fifo)) {
                    if (byte == -1) {
                        break;
                    }
                    printf("%c", byte);
                }
            }
        }
    }
}

int
main(void)
{
    Client client = client_create();
    client_run(client);
    client_destroy(client);
    return 0;
}
