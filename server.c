#include <stdlib.h>
#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell_v6.h>

#include "desktop.h"
#include "server.h"

struct mum_server *server_create(void)
{
    struct mum_server *server = calloc(1, sizeof(*server));
    if (!server) {
        fprintf(stderr, "failed to allocate mem for server\n");
        goto fail;
    }

    server->display = wl_display_create();
    server->event_loop = wl_display_get_event_loop(server->display);

    server->backend = wlr_backend_autocreate(server->display);
    if (server->backend == NULL) {
        fprintf(stderr, "could not create backend\n");
        goto fail;
    }

    server->renderer = wlr_gles2_renderer_create(server->backend);
    server->data_device_manager =
        wlr_data_device_manager_create(server->display);
    wl_display_init_shm(server->display);

    const char *socket = wl_display_add_socket_auto(server->display);
    if (socket == NULL) {
        fprintf(stderr, 
                "could not add socket, maybe too many compositors running\n");
        goto fail;
    }

    if (!wlr_backend_start(server->backend)) {
        fprintf(stderr, "could not start backend\n");
        goto fail;
    }

    server->desktop = desktop_create(server->display, server->renderer);
    if (!server->desktop) {
        fprintf(stderr, "could not create desktop\n");
        goto fail;
    }
    
    return server;

fail:
    server_destroy(server);
    return NULL;
}

void server_destroy(struct mum_server *server) {
    if (server) {
        desktop_destroy(server->desktop);

        wl_display_destroy(server->display);
        wl_event_loop_destroy(server->event_loop);

        wlr_renderer_destroy(server->renderer);
        wlr_backend_destroy(server->backend);
        wlr_data_device_manager_destroy(server->data_device_manager);
    }
    free(server);
}
