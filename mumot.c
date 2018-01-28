#include <stdlib.h>
#include <stdio.h>

#include <wayland-server.h>
#include "server.h"

int main(int argc, char *argv[])
{
    struct mum_server *server = server_create();
    if (!server) {
        fprintf(stderr, "failed to create server\n");
        return EXIT_FAILURE;
    }

    /*wl_display_run(server->display);*/

    server_destroy(server);
    return EXIT_SUCCESS;
}
