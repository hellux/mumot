struct mum_desktop {
    struct wl_list monitors; /* mum_monitor */

    struct wlr_compositor *compositor;
    struct wlr_xdg_shell_v6 *xdg_shell;

    struct wl_listener xdg_shell_listener;
};

struct mum_desktop *desktop_create(struct wl_display *display,
                                   struct wlr_renderer *renderer);

void desktop_destroy(struct mum_desktop *desktop);
