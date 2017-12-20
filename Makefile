WL_PROTOCOL = /usr/share/wayland/wayland.xml
WL_UNSTABLE = /usr/share/wayland-protocols/unstable
SERVER_SRC = server.c xdg_shell_v6_server.c

WL_SERVER_HEADER = wl_server.h
WL_CLIENT_HEADER = wl_client.h
WL_XDG_SHELL_HEADER = xdg_shell_v6_server.h

server: ${SERVER_SRC}
	wayland-scanner server-header ${WL_PROTOCOL} ${WL_SERVER_HEADER}
	wayland-scanner \
		server-header \
		${WL_UNSTABLE}/xdg-shell/xdg-shell-unstable-v6.xml \
		${WL_XDG_SHELL_HEADER}

	cc ${SERVER_SRC} -lwayland-server -o server

client: client.c
	wayland-scanner client-header ${WL_PROTOCOL} ${WL_CLIENT_HEADER}
	cc client.c -lwayland-client -o client
	make clean

clean:
	rm -f ${WL_SERVER_HEADER} ${WL_CLIENT_HEADER} ${WL_XDG_SHELL_HEADER}
