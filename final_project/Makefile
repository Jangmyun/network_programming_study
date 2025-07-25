all: client server

debug: client_debug server_debug

client: client.c console.c game.c
	gcc -o client.out client.c console.c game.c

server: server.c console.c game.c
	gcc -o server.out server.c console.c game.c

client_debug: client.c console.c game.c
	gcc -o client.out client.c console.c game.c -D DEBUG

server_debug: server.c console.c game.c
	gcc -o server.out server.c console.c game.c -D DEBUG

rm:
	rm server.out client.out server_debug.out client_debug.out