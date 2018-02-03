#include <stdlib.h>
#include <wlr/render.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell_v6.h>
#include <wlr/types/wlr_output.h>
#include "xdg_shell.h"

#include "desktop.h"

void handle_output_add(struct wl_listener *listener, void *data)
{
    struct wlr_output *output = data;
    printf("output added: %s\n", output->name);
}

struct mum_desktop *desktop_create(struct wl_display *display,
                                   struct wlr_renderer *renderer)
{
    struct mum_desktop *desktop = calloc(1, sizeof(*desktop));
    if (!desktop) {
        fprintf(stderr, "failed to allocate mem for desktop\n");
        goto fail;
    }

    wl_list_init(&desktop->monitors);

    desktop->compositor = wlr_compositor_create(display, renderer);
    if (!desktop->compositor) {
        fprintf(stderr, "failed to create compositor\n");
        goto fail;
    }

    desktop->xdg_shell = wlr_xdg_shell_v6_create(display);
    if (!desktop->xdg_shell) {
        fprintf(stderr, "failed to create xdg shell\n");
        goto fail;
    }
    wl_signal_add(&desktop->xdg_shell->events.new_surface,
                  &desktop->xdg_shell_listener);
    desktop->xdg_shell_listener.notify = handle_xdg_shell_surface;

    desktop->output_add_listener.notify = handle_output_add;

    return desktop;

fail:
    desktop_destroy(desktop);
    return NULL;
}

void desktop_destroy(struct mum_desktop *desktop)
{
    if (desktop) {
        wlr_compositor_destroy(desktop->compositor);
        wlr_xdg_shell_v6_destroy(desktop->xdg_shell);
    }

    free(desktop);
}
