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
#include <wlr/types/wlr_server_decoration.h>

#define MON_WS_COUNT 5
#define WS_NAME_SIZE 128

struct window {
    struct wlr_xdg_surface_v6 *xsurface;

    struct wl_list link; /* workspace.windows */
};

struct workspace {
    struct wl_list windows; /* window.link */
    int window_count; /* number of windows in workspace */
    int master_count; /* max number of master windows */
    double master_ratio; /* ratio of horizontal space master will use */

    char name[WS_NAME_SIZE];
};

struct monitor {
    struct wlr_output* output;

    struct workspace workspaces[MON_WS_COUNT];
    int current;

    struct wl_listener frame_listener;
    struct wl_listener destroy_listener;

    struct wl_list link; /* monitors */
};

struct wl_list monitors; /* monitor.link */

struct wl_display *display = NULL;
struct wl_event_loop *event_loop = NULL;

struct wlr_backend *backend = NULL;
struct wlr_renderer *renderer = NULL;
struct wlr_data_device_manager *data_device_manager = NULL;
struct wlr_compositor *compositor = NULL;
struct wlr_xdg_shell_v6 *xdg_shell = NULL;
struct wlr_server_decoration_manager *ssd_manager = NULL;

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

void place_windows_stack(int x, int y, int width, int height,
                         int window_count,
                         struct wlr_box boxes[])
{
    int win_height = height / window_count;
    int extra = height - window_count*win_height;

    for (int i = 0; i < window_count; i++) {
        int y_next = y + win_height;
        if (extra > 0) {
            y_next++;
            extra--;
        }

        boxes[i].x = x;
        boxes[i].y = y;
        boxes[i].width = width;
        boxes[i].height = y_next-y;

        y = y_next;
    }
}

void place_windows_master(int x, int y, int width, int height,
                          int window_count,
                          int master_count, int master_size,
                          struct wlr_box boxes[])
{
    if (window_count > master_count) {
        place_windows_stack(x, y,
                            master_size, height,
                            master_count,
                            boxes);
        place_windows_stack(master_size, y,
                            width-master_size, height,
                            window_count-master_count,
                            boxes+master_count);
    } else {
        place_windows_stack(x, y,
                            width, height,
                            window_count,
                            boxes);
    }
}

void handle_output_frame(struct wl_listener *listener, void *data)
{
    struct wlr_output *output = data;
    struct monitor *mon = wl_container_of(listener, mon, frame_listener);
    struct workspace *ws = &mon->workspaces[mon->current];

    wlr_output_make_current(output, NULL);

    wlr_renderer_begin(renderer, output);
    float color[4] = {0, 1.0, 1.0, 1.0};
    wlr_renderer_clear(renderer, &color);

    if (ws->window_count == 0)
        goto done;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    struct wlr_box *render_boxes = calloc(ws->window_count,
                                          sizeof(*render_boxes));
    place_windows_master(0, 0, output->width, output->height,
                         ws->window_count,
                         ws->master_count, output->width*ws->master_ratio,
                         render_boxes);
    struct window *win;
    int i = 0;
    wl_list_for_each(win, &ws->windows, link) {
        struct wlr_xdg_surface_v6 *xsurf = win->xsurface;
        struct wlr_surface *surf = xsurf->surface;
        struct wlr_box *render_box = &render_boxes[i++];

        printf("%d: %d, %d (%dx%d)\n", i-1,
                render_box->x, render_box->y,
                render_box->width, render_box->height);

        if (!wlr_surface_has_buffer(surf)) {
            continue;
        }

        wlr_xdg_toplevel_v6_set_size(
            xsurf, render_box->width, render_box->height);
        float matrix[16];
        wlr_matrix_project_box(&matrix, render_box,
                               surf->current->transform,
                               0, &output->transform_matrix);
        wlr_render_with_matrix(renderer, surf->texture, &matrix, 1.0f);
        wlr_surface_send_frame_done(surf, &now);
    }
    free(render_boxes);

done:
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

    struct monitor *mon = calloc(1, sizeof(*mon));
    mon->output = output;
    for (int i = 0; i < MON_WS_COUNT; i++) {
        struct workspace *ws = &mon->workspaces[i];
        wl_list_init(&ws->windows);
        ws->master_count = 2;
        ws->window_count = 0;
        ws->master_ratio = 0.5;
        snprintf(ws->name, WS_NAME_SIZE, "%d", i+1);
    }
    mon->current = 0;

    wl_list_insert(&monitors, &mon->link);

    mon->destroy_listener.notify = handle_output_destroy;
    wl_signal_add(&mon->output->events.destroy, &mon->destroy_listener);

    mon->frame_listener.notify = handle_output_frame;
    wl_signal_add(&mon->output->events.frame, &mon->frame_listener);

    wlr_output_create_global(mon->output);
}

void handle_xdg_shell_surface(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_surface_v6 *xsurf = data;
    
    printf("role: %d, title: %s, app_id: %s\n",
           xsurf->role, xsurf->title, xsurf->app_id);

    struct monitor *mon = wl_container_of(monitors.next, mon, link);

    struct window *win = calloc(1, sizeof(*win));
    win->xsurface = xsurf;

    struct workspace *ws = &mon->workspaces[mon->current];
    wl_list_insert(&ws->windows, &win->link);
    ws->window_count++;
}

void cleanup(void)
{
    wlr_server_decoration_manager_destroy(ssd_manager);
    wlr_data_device_manager_destroy(data_device_manager);
    wlr_xdg_shell_v6_destroy(xdg_shell);
    wlr_compositor_destroy(compositor);
    wlr_renderer_destroy(renderer);
    wlr_backend_destroy(backend);

    while (!wl_list_empty(&monitors)) {
        struct monitor *mon = wl_container_of(monitors.next, mon, link);
        wlr_output_destroy(mon->output);
        wl_list_remove(&mon->link);
    }

    wl_display_destroy(display);
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

    new_output_listener.notify = handle_output_add;
    wl_signal_add(&backend->events.new_output, &new_output_listener);

    if (!wlr_backend_start(backend)) {
        fprintf(stderr, "could not start backend\n");
        goto fail;
    }

    renderer = wlr_gles2_renderer_create(backend);

    const char *socket = wl_display_add_socket_auto(display);
    if (!socket) {
        fprintf(stderr, 
                "could not add socket, maybe too many compositors running\n");
        goto fail;
    }
    setenv("WAYLAND DISPLAY", socket, true);

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

    data_device_manager = wlr_data_device_manager_create(display);

    ssd_manager = wlr_server_decoration_manager_create(display);
    wlr_server_decoration_manager_set_default_mode(
        ssd_manager, WLR_SERVER_DECORATION_MANAGER_MODE_SERVER
    );

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
