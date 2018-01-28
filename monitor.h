struct mum_monitor {
    struct wl_list surfaces;
    struct wlr_output output;
    struct wl_list link; /* mum_desktop.monitors */
}
