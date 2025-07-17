all: server client

debug: server_debug client_debug

server: server.c stop_and_wait.c
	gcc -o server.out server.c stop_and_wait.c

server_debug: server.c stop_and_wait.c
	gcc -o server_debug.out server.c stop_and_wait.c -D DEBUG

client: client.c stop_and_wait.c
	gcc -o client.out client.c stop_and_wait.c

client_debug: client.c stop_and_wait.c
	gcc -o client_debug.out client.c stop_and_wait.c -D DEBUG