WL_PROTOCOL = /usr/share/wayland/wayland.xml
WL_SERVER_H = wayland_server.h
WL_C = wayland.c

WL_XDG_SHELL = /usr/share/wayland-protocols/unstable/xdg-shell
WL_XDG_SHELL_SERVER_H = xdg_shell_v6_server.h
WL_XDG_SHELL_C = xdg_shell_v6.c

WL_SRC = ${WL_C} ${WL_XDG_SHELL_C}
SERVER_SRC = mumot.c server.c xdg_shell_v6_server.c

server: ${SERVER_SRC}
	make protocols
	cc ${WL_SRC} ${SERVER_SRC} -lwayland-server -o mumot

protocols:
	wayland-scanner server-header ${WL_PROTOCOL} ${WL_SERVER_H}
	wayland-scanner code ${WL_PROTOCOL} ${WL_C}
	wayland-scanner server-header ${WL_XDG_SHELL}/xdg-shell-unstable-v6.xml \
		${WL_XDG_SHELL_SERVER_H}
	wayland-scanner code ${WL_XDG_SHELL}/xdg-shell-unstable-v6.xml \
		${WL_XDG_SHELL_C}

clean:
	rm -f ${WL_SRC}
	rm -f ${WL_SERVER_H}
	rm -f ${WL_XDG_SHELL_SERVER_H}
