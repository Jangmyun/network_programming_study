all : client server client_debug server_debug

client : clnt.c
	gcc clnt.c -o client.out
server : serv.c
	gcc serv.c -o server.out

client_debug : clnt.c
	gcc clnt.c -o client_debug.out
server_debug : serv.c
	gcc serv.c -o server_debug.out

