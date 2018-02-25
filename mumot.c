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
    struct wlr_output* output;

    struct wl_list surfaces;

    struct wl_listener frame_listener;
    struct wl_listener destroy_listener;

    struct monitor *next;
};

struct monitor *monitors;

struct wl_display *display;
struct wl_event_loop *event_loop;

struct wlr_backend *backend;
struct wlr_renderer *renderer;
struct wlr_data_device_manager *data_device_manager;
struct wlr_compositor *compositor;
struct wlr_xdg_shell_v6 *xdg_shell;

struct wl_listener new_output_listener;
struct wl_listener xdg_shell_listener;

void handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct monitor *mon = wl_container_of(listener, mon, destroy_listener);

    wl_list_remove(&mon->frame_listener.link);
    wl_list_remove(&mon->destroy_listener.link);

    if (monitors == mon) {
        monitors = mon->next;
    } else {
        struct monitor *before = monitors;
        while (before->next != mon)
            before = before->next;
        before->next = mon->next;
    }

    free(mon);
}

void handle_output_frame(struct wl_listener *listener, void *data)
{
    struct wlr_output *output = data;
    struct monitor *mon = wl_container_of(listener, mon, frame_listener);

    wlr_output_make_current(output, NULL);

    wlr_renderer_begin(renderer, output);
    float color[4] = {0, 1.0, 1.0, 1.0};
    wlr_renderer_clear(renderer, &color);
    wlr_output_swap_buffers(output, NULL, NULL);
    wlr_renderer_end(renderer);
}

void handle_output_add(struct wl_listener *listener, void *data)
{
    struct wlr_output *output = data;

    if (!wl_list_empty(&output->modes)) {
        struct wlr_output_mode *mode = wl_container_of(output->modes.prev,
                                                       mode, link);
        wlr_output_set_mode(output, mode);
    }

    struct monitor *mon = malloc(sizeof(*mon));
    wl_list_init(&mon->surfaces);
    mon->output = output;
    mon->next = NULL;

    if (monitors) {
        struct monitor *m = monitors;
        while (m->next)
            m = m->next;
        m->next = mon;
    } else {
        monitors = mon;
    }

    mon->destroy_listener.notify = handle_output_destroy;
    wl_signal_add(&mon->output->events.destroy, &mon->destroy_listener);

    mon->frame_listener.notify = handle_output_frame;
    wl_signal_add(&mon->output->events.frame, &mon->frame_listener);

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

void init(void)
{
    monitors = NULL;

    display = wl_display_create();
    event_loop = wl_display_get_event_loop(display);

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

    new_output_listener.notify = handle_output_add;
    wl_signal_add(&backend->events.new_output, &new_output_listener);

    if (!wlr_backend_start(backend)) {
        fprintf(stderr, "could not start backend\n");
        goto fail;
    }

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

    xdg_shell_listener.notify = handle_xdg_shell_surface;
    wl_signal_add(&xdg_shell->events.new_surface, &xdg_shell_listener);

    return;

fail:
    cleanup();
    exit(1);
}

int main(int argc, char *argv[])
{
    init();
    wl_display_run(display);
    cleanup();

    return EXIT_SUCCESS;
}
