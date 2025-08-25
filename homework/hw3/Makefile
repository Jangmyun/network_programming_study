all: client server

debug: client_debug server_debug

client_debug: client.c muplx.c
	gcc -o client.out client.c muplx.c -D DEBUG

server_debug: server.c muplx.c
	gcc -o server.out server.c muplx.c -D DEBUG

client: client.c muplx.c
	gcc -o client.out client.c muplx.c

server: server.c muplx.c
	gcc -o server.out server.c muplx.c