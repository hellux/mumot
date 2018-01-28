struct mum_server {
    struct mum_desktop *desktop;

    struct wl_display *display;
    struct wl_event_loop *event_loop;

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_data_device_manager *data_device_manager;
};

struct mum_server *server_create(void);
void server_destroy(struct mum_server *server);
