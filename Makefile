WL_PROTOCOL = /usr/share/wayland/wayland.xml
WL_UNSTABLE = /usr/share/wayland-protocols/unstable
SERVER_SRC = server.c xdg_shell_unstable_v6_server.c

server: ${SERVER_SRC}
	wayland-scanner server-header ${WL_PROTOCOL} wl_server.h
	wayland-scanner \
		server-header \
		${WL_UNSTABLE}/xdg-shell/xdg-shell-unstable-v6.xml \
		xdg_shell_unstable_v6_server.h

	cc ${SERVER_SRC} -lwayland-server -o server

client: client.c
	wayland-scanner client-header ${WL_PROTOCOL} wl_client.h
	cc client.c -lwayland-client -o client
	make clean

clean:
	rm -f wl_client.h wl_server.h xdg_shell_unstable_v6_server.h
