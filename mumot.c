#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render.h>
#include <wlr/render/gles2.h>
#include <wlr/render/matrix.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell_v6.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_screenshooter.h>

struct monitor {
    struct wlr_output* output;

    struct wl_list surfaces;

    struct wl_listener frame_listener;
    struct wl_listener destroy_listener;

    struct wl_list link; /* monitors */
};

struct wl_list monitors; /* monitor.link */

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
    wl_list_remove(&mon->link);

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

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    struct wl_resource *res;
    wl_resource_for_each(res, &compositor->surfaces) {
        struct wlr_surface *s = wl_resource_get_user_data(res);
        if (!wlr_surface_has_buffer(s)) {
            continue;
        }

        struct wlr_box render_box = {
            .x = 20, .y = 40,
            .width = s->current->width, .height = s->current->height
        };
        float matrix[16];
        wlr_matrix_project_box(&matrix, &render_box,
                               s->current->transform,
                               0, &output->transform_matrix);
        wlr_render_with_matrix(renderer, s->texture, &matrix, 1.0f);
        wlr_surface_send_frame_done(s, &now);
    }

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

    wl_list_insert(&monitors, &mon->link);

    mon->destroy_listener.notify = handle_output_destroy;
    wl_signal_add(&mon->output->events.destroy, &mon->destroy_listener);

    mon->frame_listener.notify = handle_output_frame;
    wl_signal_add(&mon->output->events.frame, &mon->frame_listener);

    wlr_output_create_global(mon->output);

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

    while (!wl_list_empty(&monitors)) {
        struct monitor *mon = wl_container_of(monitors.next, mon, link);
        wlr_output_destroy(mon->output);
        wl_list_remove(&mon->link);
    }
}

void init(void)
{
    wl_list_init(&monitors);

    display = wl_display_create();
    event_loop = wl_display_get_event_loop(display);

    backend = wlr_backend_autocreate(display);
    if (!backend) {
        fprintf(stderr, "could not create backend\n");
        goto fail;
    }

    renderer = wlr_gles2_renderer_create(backend);
    data_device_manager = wlr_data_device_manager_create(display);

    const char *socket = wl_display_add_socket_auto(display);
    if (!socket) {
        fprintf(stderr, 
                "could not add socket, maybe too many compositors running\n");
        goto fail;
    }
    setenv("WAYLAND DISPLAY", socket, true);

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

    wl_display_init_shm(display);
    wlr_screenshooter_create(display);
    wlr_primary_selection_device_manager_create(display);

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
