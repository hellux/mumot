SERVER_SRC = mumot.c desktop.c xdg_shell.c server.c monitor.c

server: ${SERVER_SRC}
	tcc ${SERVER_SRC} -lwayland-server -lwlroots -o mumot

clean:
	rm -f ${SERVER_SRC}
