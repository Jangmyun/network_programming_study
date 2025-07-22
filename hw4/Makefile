all: client server

client: client.c search.c console.c
	gcc -o client.out client.c search.c console.c

server: server.c search.c console.c
	gcc -o server.out server.c search.c console.c

rm:
	rm server.out client.out server_debug.out client_debug.out