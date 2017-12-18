#include <stdlib.h>
#include <stdio.h>

#include "wl_server.h"

static void create_surface(struct wl_client *client,
                           struct wl_resource *resource, uint32_t id)
{
    printf("create surface for id %d\n", id);
}

static void create_region(struct wl_client *client,
                          struct wl_resource *resource, uint32_t id)
{
    printf("create region for id %d\n", id);
}

static const struct wl_compositor_interface compositor_interface = {
    .create_surface = create_surface,
    .create_region = create_region,
};

static void compositor_bind(struct wl_client *client, void *data,
                            uint32_t version, uint32_t id)
{
    printf("compositor bind: version %d, id %d\n", version, id);

    struct wl_resource *resource;
    resource = wl_resource_create(client, &wl_compositor_interface,
                                  version, id);
    
    if (resource == NULL) {
        wl_client_post_no_memory(client);
        printf("no mem\n");
        return;
    }

    wl_resource_set_implementation(resource, &compositor_interface,
                                   data, NULL);
}

static void output_bind(struct wl_client *client, void *data,
                        uint32_t version, uint32_t id)
{
    printf("output_bind: version %d, id %d\n", version, id);
}

/* Bind interfaces to local functions.
 * interface parameters are pointer to wl_interface structs initialized in
 * server header */
void bind_functions(struct wl_display *display) {
    wl_global_create(display, &wl_compositor_interface, 1, NULL,
                     &compositor_bind);
    wl_global_create(display, &wl_output_interface, 1, NULL,
                     &output_bind);
}

int main (int argc, char **argv)
{
    struct wl_display *display;
    struct wl_event_loop *event_loop;

    display = wl_display_create();
    event_loop = wl_display_get_event_loop(display);
    const char *socket = wl_display_add_socket_auto(display);
    bind_functions(display);
    wl_display_init_shm(display);

    printf("display server on socket %d\n", socket);

    wl_display_run(display);

    return EXIT_SUCCESS;
}
