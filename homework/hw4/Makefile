all: client server

debug: client_debug server_debug

client: client.c search.c console.c
	gcc -o client.out client.c search.c console.c

server: server.c search.c console.c
	gcc -o server.out server.c search.c console.c

client_debug: client.c search.c console.c
	gcc -o client.out client.c search.c console.c -D DEBUG

server_debug: server.c search.c console.c
	gcc -o server.out server.c search.c console.c -D DEBUG

rm:
	rm server.out client.out server_debug.out client_debug.out