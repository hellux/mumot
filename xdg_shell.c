#include <stdio.h>
#include <wlr/types/wlr_xdg_shell_v6.h>
#include "xdg_shell.h"

void handle_xdg_shell_surface(struct wl_listener *listener, void *data)
{
    struct wlr_xdg_surface_v6 *surface = data;
    
    printf("role: %d, title: %s, app_id: %s\n",
           surface->role, surface->title, surface->app_id);
}

