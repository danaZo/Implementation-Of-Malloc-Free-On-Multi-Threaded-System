CC = gcc
all: server client test_server test_stack

test_server: test_server.o
	$(CC) -g test_server.c -o test_server

client: client.o
	$(CC) -g client.c -o client

server: server.o stack.o allocate.o
	$(CC)  stack.o server.o allocate.o -o server -lpthread

test_stack: test_stack.o stack.o allocate.o
	$(CC) stack.o test_stack.o allocate.o -o test_stack -lpthread			

stack.o: stack.c
	$(CC) -g -c stack.c

allocate.o: allocate.c
	$(CC) -g -c allocate.c

test_stack.o: test_stack.c 
	$(CC) -g -c test_stack.c 	

server.o: server.c
	$(CC) -g -c server.c

clean:
	rm *.o server client test_server test_stack

	