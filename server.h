/* server -> compositor interface */
struct compositor_handlers {
};

/* compositor -> server functions */
int run_server(struct compositor_handlers *handlers);
