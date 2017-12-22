#include <stdlib.h>
#include <stdio.h>

#include "wayland_server.h"
#include "xdg_shell_v6_server.h"

#include "server.h"

struct compositor {
    struct wl_list surfaces;
};

struct compositor *compositor_create() {
    struct compositor *comp = malloc(sizeof(struct compositor));
    wl_list_init(&comp->surfaces);
    return comp;
}

void *compositor_destroy(struct compositor *comp) {
    free(comp);
}

static void compositor_create_surface(struct wl_client *client,
                                      struct wl_resource *resource,
                                      uint32_t id)
{
    struct compositor *comp = wl_resource_get_user_data(resource);
    printf("create surface for id %d\n", id);

    struct wl_resource *surf_res;
    surf_res = wl_resource_create(client, &wl_surface_interface,
                                  wl_resource_get_version(resource), id);
    if (surf_res == NULL) {
        wl_resource_post_no_memory(resource);
        return;
    }
    wl_list_insert(&comp->surfaces, wl_resource_get_link(surf_res));
}

static void compositor_create_region(struct wl_client *client,
                          struct wl_resource *resource, uint32_t id)
{
    // TODO create_region
    printf("create region for id %d\n", id);
}

static void output_release(struct wl_client *client,
                           struct wl_resource *resource) {
    wl_resource_destroy(resource);
}

static const struct wl_compositor_interface compositor_interface = {
    .create_surface = &compositor_create_surface,
    .create_region = &compositor_create_region,
};

static const struct wl_output_interface output_interface = {
    .release = output_release,
};

static void compositor_bind(struct wl_client *client, void *data,
                            uint32_t version, uint32_t id)
{
    struct wl_resource *resource;
    resource = wl_resource_create(client, &wl_compositor_interface, version, id);
    if (resource == NULL) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &compositor_interface, data, NULL);
}

static void output_bind(struct wl_client *client, void *data,
                        uint32_t version, uint32_t id)
{
    printf("output_bind: version %d, id %d\n", version, id);
    struct wl_resource *resource;
    resource = wl_resource_create(client, &wl_output_interface, version, id);
    if (resource == NULL) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &output_interface, data, NULL); 
}

static void xdg_shell_bind(struct wl_client *client, void *data,
                           uint32_t version, uint32_t id)
{
    printf("xdg-shell_bind: version %d, id %d\n", version, id);
}

/* Bind interfaces to local functions.
 * Interface parameters are pointer to wl_interface structs extern declared in
 * server header, initialized in generated wayland protocol code. */
void bind_functions(struct wl_display *display, struct compositor *comp)
{
    wl_global_create(display, &wl_compositor_interface, 1, comp,
                     &compositor_bind);
    wl_global_create(display, &wl_output_interface, 1, comp,
                     &output_bind);
    wl_global_create(display, &zxdg_shell_v6_interface, 1, comp,
                     &xdg_shell_bind);
}

int run_server(struct compositor_handlers *handlers)
{
    int ret = 255;
    struct compositor *comp = compositor_create();
    struct wl_display *display;
    struct wl_event_loop *event_loop;

    display = wl_display_create();
    const char *socket = wl_display_add_socket_auto(display);

    if (socket == NULL) {
        fprintf(stderr, 
                "could not add socket, maybe too many compositors running\n");
        ret = EXIT_FAILURE;
        goto free0;
    }

    event_loop = wl_display_get_event_loop(display);
    bind_functions(display, comp);
    wl_display_init_shm(display);

    wl_display_run(display);
    ret = EXIT_SUCCESS;
free0:
    wl_display_destroy(display);
    compositor_destroy(comp);

    return ret;
}
