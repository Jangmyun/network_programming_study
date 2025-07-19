all: sender receiver

debug: sender_debug receiver_debug

sender: sender.c rudp.c file_info.c
	gcc -o sender.out sender.c rudp.c file_info.c

sender_debug: sender.c rudp.c file_info.c
	gcc -o sender_debug.out sender.c rudp.c file_info.c -D DEBUG

receiver: receiver.c rudp.c file_info.c
	gcc -o receiver.out receiver.c rudp.c file_info.c

receiver_debug: receiver.c rudp.c file_info.c
	gcc -o receiver_debug.out receiver.c rudp.c file_info.c -D DEBUG

delete:
	rm sender.out receiver.out sender_debug.out receiver_debug.out