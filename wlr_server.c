#include <stdlib.h>
#include <stdio.h>

#include <wayland-server.h>
#include <wlr/config.h>
#include <wlr/backend.h>
#include <wlr/render.h>
#include <wlr/render/gles2.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell_v6.h>

#include "wlr_server.h"

void handle_xdg_shell_surface(struct wl_listener *listener, void *data) {
    struct wlr_xdg_surface_v6 *surface = data;
    
    printf("role: %d, title: %s, app_id: %s\n", surface->role, surface->title,
           surface->app_id);
}

int run_server(struct compositor_handlers *handlers)
{
    int ret = 255; /* faulty exit if 255 returned */

    /* TODO use struct for server variables */
    struct wl_display *display;
    struct wl_event_loop *event_loop;

    struct wlr_compositor *compositor;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_data_device_manager *data_device_manager;

    struct wlr_xdg_shell_v6 *xdg_shell;
    struct wl_listener xdg_shell_listener;

    display = wl_display_create();
    event_loop = wl_display_get_event_loop(display);

    backend = wlr_backend_autocreate(display);
    if (backend == NULL) {
        fprintf(stderr, "could not create backend\n");
        goto free0;
    }

    renderer = wlr_gles2_renderer_create(backend);
    compositor = wlr_compositor_create(display, renderer);
    data_device_manager = wlr_data_device_manager_create(display);
    wl_display_init_shm(display);

    const char *socket = wl_display_add_socket_auto(display);
    if (socket == NULL) {
        fprintf(stderr, 
                "could not add socket, maybe too many compositors running\n");
        ret = EXIT_FAILURE;
        goto free1;
    }

    bool backend_success = wlr_backend_start(backend);
    if (!backend_success) {
        fprintf(stderr, "could not start backend\n");
        goto free1;
    }

    xdg_shell = wlr_xdg_shell_v6_create(display);
    wl_signal_add(&xdg_shell->events.new_surface, &xdg_shell_listener);
    xdg_shell_listener.notify = handle_xdg_shell_surface;

    wl_display_run(display);

    ret = EXIT_SUCCESS;

free1:
    wlr_backend_destroy(backend);
free0:
    wl_display_destroy(display);

    return ret;
}
