#include "server.h"
#include <stdio.h>

void
test_server_1(void)
{
    printf("TEST_SERVER_1\n");
    Server server = server_create(30, 10, 3);
    printf("Created\n");
    server_destroy(server);
    printf("Destroyed\n");
}

void
test_server_2(void)
{
    printf("TEST_SERVER_2\n");
    Server server = server_create(30, 10, 3);
    printf("Created\n");
    server_run(server);
    server_destroy(server);
    printf("Destroyed\n");
}

int
main(void)
{
    test_server_1();
    test_server_2();
    return 0;
}
