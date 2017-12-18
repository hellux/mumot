CC = cc
WL_PROTOCOL = /usr/share/wayland/wayland.xml

server: server.c
	wayland-scanner server-header ${WL_PROTOCOL} wl_server.h
	${CC} server.c -lwayland-server -o server
	make clean

client: client.c
	wayland-scanner client-header ${WL_PROTOCOL} wl_client.h
	${CC} client.c -lwayland-client -o client
	make clean

clean:
	rm -f wl_client.h wl_server.h
