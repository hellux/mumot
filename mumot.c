#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render.h>
#include <wlr/render/gles2.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell_v6.h>
#include <wlr/types/wlr_compositor.h>

struct monitor {
    struct wl_list surfaces;
    struct wlr_output* output;

    struct monitor *next;
};

struct monitor *monitors;

struct wl_display *display;
struct wl_event_loop *event_loop;

struct wlr_backend *backend;
struct wlr_renderer *renderer;
struct wlr_data_device_manager_create *data_device_manager;
struct wlr_compositor *compositor;
struct wlr_xdg_shell_v6 *xdg_shell;

struct wl_listener xdg_shell_listener;
struct wl_listener output_add_listener;

void handle_output_add(struct wl_listener *listener, void *data)
{
    struct wlr_output *output = data;

    struct monitor *mon = malloc(sizeof(*mon));
    mon->output = output;
    wl_list_init(&mon->surfaces);

    if (monitors) {
        struct monitor *m = monitors;
        while (m->next)
            m = m->next;
        m->next = mon;
    } else {
        monitors = mon;
    }

    printf("output added: %s\n", output->name);
}

void handle_xdg_shell_surface(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_surface_v6 *surface = data;
    
    printf("role: %d, title: %s, app_id: %s\n",
           surface->role, surface->title, surface->app_id);
}

void cleanup(void)
{
    wl_display_destroy(display);
    wlr_renderer_destroy(renderer);
    wlr_xdg_shell_v6_destroy(xdg_shell);

    struct monitor* m = monitors;
    while (m) {
        wlr_output_destroy(m->output);

        struct monitor* next = m->next;
        free(m);
        m = next;
    }
}

void init_server(void)
{
    assert(display = wl_display_create());
    assert(event_loop = wl_display_get_event_loop(display));

    backend = wlr_backend_autocreate(display);
    if (!backend) {
        fprintf(stderr, "could not create backend\n");
        goto fail;
    }

    renderer = wlr_gles2_renderer_create(backend);
    data_device_manager = wlr_data_device_manager_create(display);
    wl_display_init_shm(display);

    const char *socket = wl_display_add_socket_auto(display);
    if (!socket) {
        fprintf(stderr, 
                "could not add socket, maybe too many compositors running\n");
        goto fail;
    }

    if (!wlr_backend_start(backend)) {
        fprintf(stderr, "could not start backend\n");
        goto fail;
    }

    monitors = NULL;

    compositor = wlr_compositor_create(display, renderer);
    if (!compositor) {
        fprintf(stderr, "failed to create compositor\n");
        goto fail;
    }

    xdg_shell = wlr_xdg_shell_v6_create(display);
    if (!xdg_shell) {
        fprintf(stderr, "failed to create xdg shell\n");
        goto fail;
    }

    wl_signal_add(&xdg_shell->events.new_surface,
                  &xdg_shell_listener);
    xdg_shell_listener.notify = handle_xdg_shell_surface;
    output_add_listener.notify = handle_output_add;

    return;

fail:
    fflush(stdout);
    cleanup();
    exit(1);
}

int main(int argc, char *argv[])
{
    init_server();
    wl_display_run(display);
    cleanup();

    return EXIT_SUCCESS;
}
