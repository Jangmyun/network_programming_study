all: sender receiver

debug: sender_debug receiver_debug

sender: sender.c stop_and_wait.c file_info.c
	gcc -o sender.out sender.c stop_and_wait.c file_info.c

sender_debug: sender.c stop_and_wait.c file_info.c
	gcc -o sender_debug.out sender.c stop_and_wait.c file_info.c -D DEBUG

receiver: receiver.c stop_and_wait.c file_info.c
	gcc -o receiver.out receiver.c stop_and_wait.c file_info.c

receiver_debug: receiver.c stop_and_wait.c file_info.c
	gcc -o receiver_debug.out receiver.c stop_and_wait.c file_info.c -D DEBUG