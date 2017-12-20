#include "xdg_shell_v6_server.h"

/* Below functions are documented in xdg-shell-unstable-v6.xml protocol
 * file. */

void surface_destroy(struct wl_client *client, struct wl_resource *resource)
{
    
}

void surface_get_toplevel(struct wl_client *client,
                          struct wl_resource *resource, uint32_t id)
{
    
}

void surface_get_popup(struct wl_client *client, struct wl_resource *resource,
                       uint32_t id, struct wl_resource *parent,
                       struct wl_resource *positioner)
{

}

void surface_set_window_geometry(struct wl_client *client,
                                 struct wl_resource *resource,
                                 int32_t x, int32_t y,
                                 int32_t width, int32_t height)
{

}

void surface_ack_configure(struct wl_client *client,
                           struct wl_resource *resource, uint32_t serial)
{

}

void toplevel_destroy(struct wl_client *client, struct wl_resource *resource)
{

}

void toplevel_set_parent(struct wl_client *client,
                         struct wl_resource *resource,
                         struct wl_resource *parent)
{

}
                         

void toplevel_set_title(struct wl_client *client, struct wl_resource *resource,
                        const char *title)
{
    
}

void toplevel_app_id(struct wl_client *client, struct wl_resource *resource,
                     const char *app_id)
{

}

void toplevel_show_window_menu(struct wl_client *client,
                               struct wl_resource *resource,
                               struct wl_resource *seat,
                               uint32_t serial, int32_t x, int32_t y)
{

}

void toplevel_move(struct wl_client *client,
                   struct wl_resource *resource,
                   struct wl_resource *seat,
                   uint32_t serial)
{

}

void toplevel_resize(struct wl_client *client,
                     struct wl_resource *resource,
                     struct wl_resource *seat,
                     uint32_t serial, uint32_t edges)
{

}

void toplevel_set_max_size(struct wl_client *client,
                           struct wl_resource *resource,
                           int32_t width, int32_t height)
{

}

void toplevel_set_min_size(struct wl_client *client,
                           struct wl_resource *resource,
                           int32_t width, int32_t height)
{

}

void toplevel_set_maximized(struct wl_client *client,
                            struct wl_resource *resource)
{

}

void toplevel_unset_maximized(struct wl_client *client,
                              struct wl_resource *resource)
{

}

void toplevel_set_fullscreen(struct wl_client *client,
                             struct wl_resource *resource,
                             struct wl_resource *output)
{

}

void toplevel_unset_fullscreen(struct wl_client *client,
                               struct wl_resource *resource)
{

}

void toplevel_set_minimized(struct wl_client *client,
                            struct wl_resource *resource)
{

}

struct zxdg_surface_v6_interface surface_interface = {
    .destroy = surface_destroy,
    .get_toplevel = surface_get_toplevel,
    .get_popup = surface_get_popup,
    .set_window_geometry = surface_set_window_geometry,
    .ack_configure = surface_ack_configure,
};

struct zxdg_toplevel_v6_interface toplevel_interface = {
    .destroy = toplevel_destroy,
    .set_parent = toplevel_set_parent,
    .set_title = toplevel_set_title,
    .set_app_id = toplevel_app_id,
    .show_window_menu = toplevel_show_window_menu,
    .move = toplevel_move,
    .resize = toplevel_resize,
    .set_max_size = toplevel_set_max_size,
    .set_min_size = toplevel_set_min_size,
    .set_maximized = toplevel_set_maximized,
    .unset_maximized = toplevel_unset_maximized,
    .set_fullscreen = toplevel_set_fullscreen,
    .unset_fullscreen = toplevel_unset_fullscreen,
    .set_minimized = toplevel_set_minimized,
};
