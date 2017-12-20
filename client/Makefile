WL_PROTOCOL = /usr/share/wayland/wayland.xml
WL_CLIENT_H = wayland_client.h
WL_C = wayland.c

WL_XDG_SHELL = /usr/share/wayland-protocols/unstable/xdg-shell
WL_XDG_SHELL_CLIENT_H = xdg_shell_v6_client.h
WL_XDG_SHELL_C = xdg_shell_v6.c

WL_SRC = ${WL_C} ${WL_XDG_SHELL_C}
CLIENT_SRC = client.c

client: ${CLIENT_SRC}
	make protocols
	cc client.c -lwayland-client -o client

protocols:
	wayland-scanner client-header ${WL_PROTOCOL} ${WL_CLIENT_H}
	wayland-scanner code ${WL_PROTOCOL} ${WL_C}
	wayland-scanner client-header ${WL_XDG_SHELL}/xdg-shell-unstable-v6.xml \
		${WL_XDG_SHELL_CLIENT_H}
	wayland-scanner code ${WL_XDG_SHELL}/xdg-shell-unstable-v6.xml \
		${WL_XDG_SHELL_C}

clean:
	rm -f ${WL_SRC}
	rm -f ${WL_CLIENT_H}
	rm -f ${WL_XDG_SHELL_CLIENT_H}
