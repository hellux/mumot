#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wayland_client.h"

struct wl_display *display = NULL;
struct wl_compositor *compositor = NULL;

/* called when global is added to server registry */
static void registry_global_add(void *data, struct wl_registry *registry,
                                uint32_t id, const char *interface,
                                uint32_t version)
{
    printf("registry event for %s, id: %d\n", interface, id);
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, id,
                                      &wl_compositor_interface, 1);
    }
}

/* called when global is removed from server registry */
static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t id)
{
    printf("registry losing event for %d\n", id);
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global_add,
    .global_remove = registry_global_remove,
};

int main(int argc, char **argv)
{
    display = wl_display_connect(NULL);
    if (display == NULL) {
        fprintf(stderr, "can't connect to display server\n");
        return EXIT_FAILURE;
    }
    printf("connected to display server\n");

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_dispatch(display);
    wl_display_roundtrip(display); /* lock until server response */

    wl_registry_bind(registry, 1, &wl_output_interface, 1);
    wl_display_roundtrip(display);

    wl_display_disconnect(display);
    printf("disconnected from display server\n");
    
    return EXIT_SUCCESS;
}
